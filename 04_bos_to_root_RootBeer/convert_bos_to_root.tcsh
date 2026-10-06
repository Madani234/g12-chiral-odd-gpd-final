#!/bin/tcsh
#=============================================================================
#  convert_bos_to_root.tcsh
#-----------------------------------------------------------------------------
#  CE QU'IL FAIT
#    Convertit les ~60 000 fichiers BOS de g12 (skim 2-2pos1neg_not_1ckaon1ctrk)
#    en fichiers ROOT (DST) via la macro RootBeer rbtest.C, par lots de 50,
#    puis fusionne le tout en un seul g12_Total.root.
#
#  OÙ LE LANCER
#    Sur l'hôte ifarm, dans un shell tcsh.
#      - PAS dans un conteneur (Apptainer ou autre)
#      - PAS en bash : l'environnement RootBeer est écrit en csh (cf. étape 3)
#
#  PRÉREQUIS
#    - RootBeer 6.0 compilé dans $RBDIR (ci-dessous)
#    - Correction slib appliquée (cf. étape 4, à faire une seule fois)
#    - rbtest.C présent dans $RBDIR (c'est lui qui contient le filtre de topologie)
#
#  UTILISATION (une étape à la fois, pour pouvoir contrôler entre chaque)
#    tcsh convert_bos_to_root.tcsh prepare   # étapes 1-2 : liste maîtresse + découpage
#    tcsh convert_bos_to_root.tcsh test      # étape 5    : conversion d'une seule liste
#    tcsh convert_bos_to_root.tcsh run       # étape 6    : boucle complète (reprend là où elle s'est arrêtée)
#    tcsh convert_bos_to_root.tcsh check     # étape 7    : liste les lots en timeout
#    tcsh convert_bos_to_root.tcsh retry     # étape 7    : efface les lots en timeout, puis relancer "run"
#    tcsh convert_bos_to_root.tcsh merge     # étape 9    : hadd -> g12_Total.root
#=============================================================================


#-----------------------------------------------------------------------------
#  CHEMINS
#  /!\ Toujours écrire dans /work, JAMAIS dans /cache : /cache est en lecture
#      seule (archive adossée aux bandes). /cache ne sert qu'à LIRE les BOS.
#-----------------------------------------------------------------------------
set BOS_SRC  = /cache/clas/g12/production/pass1/bos/2-2pos1neg_not_1ckaon1ctrk
set LISTES   = /work/clas/clasg12/madani/lists50
set SORTIES  = /work/clas/clasg12/madani/roots50
set RBDIR    = /work/clas/clasg12/madani/rootbeer6.0
set FUSION   = /work/clas/clasg12/madani/Merged_Total

set FILELIST = $LISTES/g12_filelist.txt
set MACRO    = rbtest.C
set TOTAL    = $FUSION/g12_Total.root


#-----------------------------------------------------------------------------
#  Choix de l'étape
#-----------------------------------------------------------------------------
if ( $#argv != 1 ) then
    echo "Usage : tcsh $0 {prepare|test|run|check|retry|merge}"
    exit 1
endif
set MODE = $argv[1]

mkdir -p $LISTES $SORTIES $FUSION


#=============================================================================
#  ÉTAPES 1-2 : LISTE MAÎTRESSE ET DÉCOUPAGE
#=============================================================================
if ( $MODE == "prepare" ) then

    #-------------------------------------------------------------------------
    #  Étape 1 — la liste maîtresse
    #  Tous les fichiers BOS du skim (extensions .A00, .A01, ...).
    #  Attendu : ~59 973 entrées.
    #-------------------------------------------------------------------------
    ls $BOS_SRC/*.A?? > $FILELIST
    echo "Liste maîtresse : `wc -l < $FILELIST` fichiers BOS -> $FILELIST"

    #-------------------------------------------------------------------------
    #  Étape 2 — le découpage en lots de 50
    #  -l 50   : 50 fichiers BOS par liste
    #  -d      : suffixe numérique (0000, 0001, ...)
    #  -a 4    : 4 chiffres -> liste_0000 ... liste_1199 (~1200 listes)
    #
    #  POURQUOI 50 : constat empirique. Avec des listes de ~600 fichiers,
    #  ~18 % des lots finissaient en timeout et on perdait ~16 % des
    #  événements. Des lots de 50 sont assez courts pour ne (presque) jamais
    #  expirer, et un échec ne coûte que 50 fichiers à relancer.
    #-------------------------------------------------------------------------
    cd $LISTES
    split -l 50 -d -a 4 $FILELIST liste_
    echo "Découpage : `ls $LISTES/liste_* | wc -l` listes créées dans $LISTES"
    exit 0
endif


#=============================================================================
#  ÉTAPE 3 : ENVIRONNEMENT ROOTBEER  (nécessaire pour test / run / retry)
#-----------------------------------------------------------------------------
#  Doit être sourcé depuis tcsh : en bash, la compilation de la macro échoue
#  sur « RootBeerUtil.h not found » car rootbeer.cshrc (syntaxe csh) n'a pas
#  pu définir les chemins d'inclusion. -> Rester en tcsh.
#=============================================================================
if ( $MODE == "test" || $MODE == "run" ) then
    cd $RBDIR
    setenv ROOTBEER `pwd`
    source scripts/rootbeer.cshrc

    if ( ! -e $MACRO ) then
        echo "ERREUR : $MACRO introuvable dans `pwd`"
        exit 1
    endif
endif


#=============================================================================
#  ÉTAPE 4 : CORRECTION slib  (EN COMMENTAIRE — à faire UNE fois, à la main)
#-----------------------------------------------------------------------------
#    cp -d $ROOTBEER/slib/Linux/* $ROOTBEER/slib/Linux64RHEL9/
#
#  POURQUOI : à la compilation, le script uname_local renvoie « Linux », donc
#  les bibliothèques partagées (libRootBeer.so, ...) sont rangées dans
#  slib/Linux. À l'exécution sur AlmaLinux 9, RootBeer les cherche dans
#  slib/Linux64RHEL9 -> la bibliothèque ne se charge pas -> cascade de
#  « unresolved symbol » (getNextFile, createBeerObject, TAGR, PART, SCR...).
#
#  NB : utiliser cp -d et non ln -s. Linux64RHEL9 existe déjà comme vrai
#  répertoire : un ln -s crée un lien circulaire À L'INTÉRIEUR de celui-ci.
#
#  /!\ La cause n'est PAS corrigée à la source : après TOUTE recompilation
#      de RootBeer (make), il faut refaire ce cp -d.
#=============================================================================


#=============================================================================
#  ÉTAPE 5 : TEST SUR UNE SEULE LISTE  (à faire avant de lancer les ~1200)
#-----------------------------------------------------------------------------
#  Signature : rbtest(nÉvénements, entrée, sortie)
#     nÉvénements = 0      -> lire TOUS les événements
#     entrée = "-L<liste>" -> le préfixe -L indique une LISTE de fichiers BOS
#                             (sans -L, rbtest attend un seul fichier BOS ;
#                              un répertoire fait planter la macro)
#     sortie               -> fichier ROOT produit
#
#  Le filtre de topologie (p = 14, π⁺ = 8, π⁻ = 9, γ = 1, codes GEANT) est
#  dans rbtest.C, PAS dans ce script. Pour changer de topologie, modifier
#  la macro.
#=============================================================================
if ( $MODE == "test" ) then
    set f   = $LISTES/liste_0000
    set out = $SORTIES/g12_0000.root
    set log = $SORTIES/g12_0000.log

    rootbeer -b -q 'rbtest.C(0,"-L'$f'","'$out'")' >& $log

    echo "Test terminé. Vérifier :"
    echo "   ls -lh $out"
    echo "   tail $log"
    echo "Si tout est bon : tcsh $0 run  (g12_0000 sera sauté, déjà fait)"
    exit 0
endif


#=============================================================================
#  ÉTAPE 6 : BOUCLE COMPLÈTE
#-----------------------------------------------------------------------------
#  Un .root + un .log par lot :  liste_0000 -> g12_0000.root + g12_0000.log
#
#  if ( -e $out ) continue : un lot dont le .root existe déjà est sauté.
#  On peut donc relancer "run" autant de fois que nécessaire sans réécraser
#  les lots déjà réussis (seuls les lots effacés par "retry" sont refaits).
#
#  /!\ SYNTAXE : ne PAS écrire "rbtest.C(0,\"-L$f\",\"$out\")".
#      En csh, \" à l'intérieur de "..." ne marche pas -> « Unmatched '"' ».
#      On alterne apostrophes (pour protéger les ") et fin d'apostrophe
#      (pour laisser le shell développer $f et $out), comme ci-dessous.
#=============================================================================
if ( $MODE == "run" ) then
    foreach f ( $LISTES/liste_* )
        set n   = `basename $f | sed 's/liste_//'`
        set out = $SORTIES/g12_$n.root
        set log = $SORTIES/g12_$n.log

        if ( -e $out ) continue

        echo "Lot $n ..."
        rootbeer -b -q 'rbtest.C(0,"-L'$f'","'$out'")' >& $log
    end
    echo "Boucle terminée. Lancer : tcsh $0 check"
    exit 0
endif


#=============================================================================
#  ÉTAPE 7 : DÉTECTION DES ÉCHECS
#-----------------------------------------------------------------------------
#  Un lot en échec se reconnaît à « timed out » dans son journal.
#
#  Le cycle, à répéter jusqu'à zéro timeout :
#     run  ->  check  ->  retry  ->  run  ->  check  ->  ...
#
#  Historique : ce cycle a permis de récupérer 65 fichiers manquants.
#  Un seul lot est resté irrécupérable : g12_0688 (fichier ROOT « zombie »,
#  c.-à-d. illisible/non fermé). Ne pas boucler indéfiniment dessus : il faut
#  l'écarter à la main avant la fusion (cf. étape 9).
#=============================================================================
if ( $MODE == "check" || $MODE == "retry" ) then
    set rates = ( `grep -l "timed out" $SORTIES/*.log` )

    if ( $#rates == 0 ) then
        echo "Aucun timeout. Passer au contrôle d'intégrité (étape 8) puis à la fusion."
        exit 0
    endif

    echo "$#rates lot(s) en timeout :"
    foreach l ( $rates )
        echo "   $l"
    end

    if ( $MODE == "retry" ) then
        # On efface .root ET .log des lots en échec : le .root partiel
        # bloquerait sinon la relance (if -e $out continue).
        foreach l ( $rates )
            rm -f $l:r.root $l
        end
        echo "Lots effacés. Relancer : tcsh $0 run"
    endif
    exit 0
endif


#=============================================================================
#  ÉTAPE 8 : CONTRÔLE D'INTÉGRITÉ  (EN COMMENTAIRE)
#-----------------------------------------------------------------------------
#  Zéro timeout ne garantit pas que tous les .root sont sains : un fichier
#  peut être tronqué ou zombie sans que le journal le signale.
#  AVANT de fusionner, relire chaque .root produit (ouverture, présence de
#  l'arbre, nombre d'entrées) pour repérer les tronqués, et noter le nombre
#  total d'événements : il servira de référence pour vérifier la fusion.
#  (La macro dédiée n'est plus fournie ici.)
#=============================================================================


#=============================================================================
#  ÉTAPE 9 : FUSION
#-----------------------------------------------------------------------------
#  Tous les g12_*.root -> $FUSION/g12_Total.root (~60 Go).
#  Retirer au préalable les fichiers repérés comme zombies/tronqués
#  (notamment g12_0688) de $SORTIES, sinon hadd s'arrête ou les ignore.
#  Étape longue : plusieurs minutes de silence après « Target path » sont
#  normales.
#=============================================================================
if ( $MODE == "merge" ) then
    if ( -e $TOTAL ) then
        echo "ERREUR : $TOTAL existe déjà. Le déplacer ou le supprimer d'abord."
        exit 1
    endif

    hadd $TOTAL $SORTIES/g12_*.root

    echo "Fusion terminée :"
    ls -lh $TOTAL
    exit 0
endif


echo "Mode inconnu : $MODE"
echo "Usage : tcsh $0 {prepare|test|run|check|retry|merge}"
exit 1


#=============================================================================
#  RAPATRIEMENT DU FICHIER FUSIONNÉ  (à lancer depuis le PC, pas depuis ifarm)
#-----------------------------------------------------------------------------
#  /work n'est visible que sur ifarm : on rebondit par login.jlab.org
#  (mot de passe + OTP), puis ifarm (mot de passe seul).
#
#    cd <dossier_de_destination>
#    scp -J madani@login.jlab.org madani@ifarm:/work/clas/clasg12/madani/Merged_Total/g12_Total.root .
#
#  RAPPELS :
#   - Se placer D'ABORD dans le dossier de destination : le « . » final
#     désigne le répertoire courant (sinon le fichier atterrit dans ~).
#   - Hôte : ifarm, pas ifarm.jlab.org.
#   - JAMAIS d'expansion d'accolades {a,b} dans un chemin distant : scp
#     moderne passe par SFTP et ne développe pas ces métacaractères.
#     Un fichier par commande.
#   - ~60 Go d'un bloc : Globus est plus robuste (reprise automatique).
#=============================================================================
