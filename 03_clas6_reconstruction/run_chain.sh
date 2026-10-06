#!/bin/bash
# =============================================================================
#  run_chain.sh — chaîne de simulation et reconstruction CLAS6 pour g12
#
#  Enchaîne les quatre étapes qui transforment le fichier BOS produit par
#  BOSwrite en un DST lisible par RootBeer :
#
#      .evt  --[gsim_bat]-->  --[gpp]-->  --[a1c]-->  --[mkdst]-->  DST
#
#  Recette de reconstruction : Josh Bryce (University of York), contact g12
#  (email du 16 juillet 2026). Les étapes gsim_bat, gpp et a1c reprennent ses
#  commandes ; l'étape mkdst n'en fait pas partie.
#
#  Détail des flags a1c et de la chaîne MC g12 :
#         https://clasweb.jlab.org/rungroups/g12/wiki/index.php/Software#Monte_Carlo_Generation_and_Reconstruction
#
#  ---------------------------------------------------------------------------
#  À LIRE AVANT DE LANCER
#  ---------------------------------------------------------------------------
#  1) Ce script tourne À L'INTÉRIEUR du conteneur CLAS6, pas sur l'hôte ifarm.
#     Entrer dans le conteneur d'abord :
#
#         apptainer shell /group/clas/builds/centos79-clas6.sif
#
#     L'hôte ifarm est en tcsh, l'intérieur du conteneur en bash : ce script
#     est en bash, il ne tournera pas si vous le lancez depuis l'hôte.
#
#  2) Les deux cartes de configuration (ffread.g12, prlink_tg-90pm30.bos) ne
#     sont PAS dans le dépôt : elles n'ont pas pu être rapatriées avant la
#     perte des accès. Il faut les récupérer sur ifarm dans
#     /work/clas/clasg12/madani/gen/ — voir le README de ce dossier.
#
#  3) Les lignes gsim_bat, gpp et a1c sont celles de Josh Bryce (seuls les
#     noms de fichiers changent). 
#
# =============================================================================

set -e   # s'arrête à la première erreur au lieu d'enchaîner sur un fichier absent

# -----------------------------------------------------------------------------
# Paramètres
# -----------------------------------------------------------------------------
WORKDIR=/work/clas/clasg12/madani/gen     # répertoire de travail sur ifarm

EVT_IN=MonteCarlo_Two_To_Three.evt        # entrée : sortie de BOSwrite (.evt)
GSIM_OUT=gsim_out.bos                      # sortie de la simulation GEANT
GPP_OUT=gpp_out.bos                        # sortie du post-processing
COOKED_OUT=cooked_out.bos                  # sortie de la reconstruction (a1c)
DST_OUT=sim_dst_final.bos                  # sortie finale, lisible par RootBeer

FFREAD_CARD=ffread.g12                     # carte de configuration de gsim
PRLINK_CARD=prlink_tg-90pm30.bos           # constantes liées à la position de cible

NTRIG=10000000                             # valeur -trig utilisée par Josh

cd "$WORKDIR"

# -----------------------------------------------------------------------------
# Environnement CLAS6 — obligatoire, à faire une fois entré dans le conteneur
# (les trois lignes données par Josh Bryce, dans cet ordre)
# -----------------------------------------------------------------------------
source /etc/profile.d/modules.sh
source /opt/environment.sh
export CLAS_CALDB_RUNINDEX=calib_user.RunIndexg12

# Vérification des fichiers indispensables : mieux vaut échouer ici avec un
# message clair qu'au milieu de la chaîne avec une erreur obscure.
for f in "$EVT_IN" "$FFREAD_CARD" "$PRLINK_CARD"; do
    if [ ! -f "$f" ]; then
        echo "ERREUR : fichier manquant -> $f"
        echo "  (les cartes ffread/prlink ne sont pas dans le dépôt, cf. README)"
        exit 1
    fi
done

# =============================================================================
#  ÉTAPE 1 — gsim_bat : simulation du détecteur
# =============================================================================
#  Fait passer les événements générés à travers une description GEANT du
#  détecteur CLAS : chaque particule traverse les sous-détecteurs et y dépose
#  de l'énergie. La sortie contient les signaux bruts, pas les trajectoires.
#
# =============================================================================

gsim_bat -ffread "$FFREAD_CARD" -trig "$NTRIG" \
         -mcin "$EVT_IN" -kine 1 -bosout "$GSIM_OUT"

# =============================================================================
#  ÉTAPE 2 — gpp : post-processing
# =============================================================================
#  gsim simule un détecteur parfait. gpp le dégrade pour le rendre réaliste :
#  il élargit les résolutions et désactive les canaux qui étaient morts pendant
#  la vraie prise de données.
#
# =============================================================================

gpp -Y -R56855 -P0x7f -o"$GPP_OUT" "$GSIM_OUT"

# =============================================================================
#  ÉTAPE 3 — a1c : reconstruction (« cooking »)
# =============================================================================
#  Programme de reconstruction de CLAS6 : il repart des signaux du détecteur
#  pour retrouver les trajectoires, les impulsions et l'identité des
#  particules.
#
# =============================================================================

a1c -T4 -sa -ct1930 -cm0 -cp0 -X0 -d1 -F -P0x1bff \
    -z0,0,-90 -A"$PRLINK_CARD" -o"$COOKED_OUT" "$GPP_OUT"

# =============================================================================
#  ÉTAPE 4 — mkdst : mise au bon format
# =============================================================================
#  Cette étape ne figure pas dans la recette de simulation ; elle vient
#  du travail propre à ce projet.
#
#  La sortie brute d'a1c est physiquement correcte mais fait planter RootBeer.
#  mkdst l'allège (suppression des bancs internes à la simulation et des bancs
#  de bas niveau) et saute le pseudo-événement que a1c écrit en tête de fichier.
#
#  mkdst doit avoir été compilé au préalable : voir mkdst/build_mkdst.sh
#
# =============================================================================

./mkdst -s1 "$COOKED_OUT" "$DST_OUT"