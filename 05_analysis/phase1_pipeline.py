import ROOT
import numpy as np
import os

ROOT.gInterpreter.Declare(r"""
double CosThetaHel(const TLorentzVector& pip, const TLorentzVector& pim) {
    TLorentzVector rho = pip + pim;
    TVector3 b = rho.BoostVector();
    TLorentzVector p = pip;
    p.Boost(-b);
    return p.Vect().Unit().Dot(b.Unit());
}
""")

FICHIER = "g12_Total.root"

_f = ROOT.TFile.Open(FICHIER)
TREE = next(k.GetName() for k in _f.GetListOfKeys() if k.GetClassName() == "TTree")
_f.Close()

Data = ROOT.RDataFrame(TREE, FICHIER)

Data = Data.Define("M_Rho",
    "sqrt((piMinus.E() + piPlus.E())*(piMinus.E() + piPlus.E())"
    " - (piMinus.Px() + piPlus.Px())*(piMinus.Px() + piPlus.Px())"
    " - (piMinus.Py() + piPlus.Py())*(piMinus.Py() + piPlus.Py())"
    " - (piMinus.Pz() + piPlus.Pz())*(piMinus.Pz() + piPlus.Pz()))"
)
Data = Data.Define("miss_m_tot2", "miss_m_tot*miss_m_tot")
Data = Data.Define("mt", "-t")
Data = Data.Define("mtprime", "-t_prime")
Data = Data.Define("muprime", "-u_prime")
Data = Data.Define("M2_ppip", "mpipprot*mpipprot")
Data = Data.Define("M2_ppim", "mpimprot*mpimprot")
Data = Data.Define("cosThetaHel", "CosThetaHel(piPlus, piMinus)")
Data = Data.Define("M2_gammarho", "mpipphot*mpipphot + mpimphot*mpimphot + mpipi*mpipi - 2*0.13957 *0.13957")
Data = Data.Define("xi", "M2_gammarho / (2*(s - 0.93827*0.93827) - M2_gammarho)")

Data = Data.Define("P_prot",     "proton.P()")
Data = Data.Define("Theta_prot", "proton.Theta()")
Data = Data.Define("Phi_prot",   "proton.Phi()")
Data = Data.Define("P_pip",      "piPlus.P()")
Data = Data.Define("Theta_pip",  "piPlus.Theta()")
Data = Data.Define("Phi_pip",    "piPlus.Phi()")
Data = Data.Define("P_pim",      "piMinus.P()")
Data = Data.Define("Theta_pim",  "piMinus.Theta()")
Data = Data.Define("Phi_pim",    "piMinus.Phi()")
Data = Data.Define("P_phot",     "photon.P()")
Data = Data.Define("Theta_phot", "photon.Theta()")
Data = Data.Define("Phi_phot",   "photon.Phi()")


Data_Exlusif = Data.Filter("abs(miss_m_tot) < 0.2 && abs(miss_E_tot) < 0.2")
Data_Rho     = Data_Exlusif.Filter("M_Rho > 0.1 && M_Rho < 1.1")
Data_pi      = Data_Rho.Filter(
    "!(mpimprot < 1.80) && "
    "!(mpipprot > 0 && mpipprot < 1.35)")
Data_xi      = Data_pi.Filter("xi < 0.33 && xi > 0.04")
Data_final   = Data_xi.Filter("-u_prime > 1 && -t_prime > 1  &&  -t< 0.5  && M2_gammarho > 2")


Histo_Rho   = Data.Histo1D(("M_Rho_raw", "M_{#pi#pi} avant coupures; M (GeV); Counts", 500, 0.2, 1.2), "M_Rho")

h_M2_raw    = Data.Histo1D(("M2_raw",   "M^{2}_{#gamma#rho} brut; M^{2} (GeV^{2}); Counts", 100, 0, 15), "M2_gammarho")
h_M2_final  = Data_final.Histo1D(("M2_final", "M^{2}_{#gamma#rho} apres coupures; M^{2} (GeV^{2}); Counts", 100, 0, 15), "M2_gammarho")
h_xi_row    = Data.Histo1D(("xi_row", "#xi avant coupures; #xi; Counts", 50, 0, 0.5), "xi")
h_xi_final  = Data_final.Histo1D(("xi_final", "#xi apres coupures; #xi; Counts", 50, 0, 0.5), "xi")
h_t_final   = Data_final.Histo1D(("t_final",  "-t apres coupures; -t (GeV^{2}); Counts", 50, -0.5, 0), "t")
h_rho_final = Data_final.Histo1D(("M_Rho_final", "M_{#pi#pi} apres coupures; M (GeV); Counts", 25, 0.1, 1.1), "M_Rho")

h_miss_raw    = Data.Histo1D(("miss_raw", "Masse manquante brute; miss_m_tot^2 (GeV^2); Counts", 200, -1, 1), "miss_m_tot2")
h_Emiss_raw   = Data.Histo1D(("Emiss_raw", "Energie manquante brute; miss_E_tot (GeV); Counts", 200, -1, 1), "miss_E_tot")
h_miss_final  = Data_Exlusif.Histo1D(("miss_final", "Masse manquante apres coupure; miss_m_tot^2 (GeV^2); Counts", 200, -1, 1), "miss_m_tot2")
h_Emiss_final = Data_Exlusif.Histo1D(("Emiss_final", "Energie manquante apres coupure; miss_E_tot (GeV); Counts", 200, -1, 1), "miss_E_tot")

h_mpipprot = Data.Histo1D(("mpipprot", "m_{p#pi^{+}}; m (GeV); Counts", 200, 1.0, 2.0), "mpipprot")
h_mpimprot = Data.Histo1D(("mpimprot", "m_{p#pi^{-}}; m (GeV); Counts", 200, 1.0, 2.0), "mpimprot")

h_M2_vs_xi = Data_final.Histo2D(("M2_vs_xi", "M^{2}_{#gamma#rho} vs #xi; #xi; M^{2}_{#gamma#rho} (GeV^{2})", 10, 0.10, 0.35, 10, 2.0, 5), "xi", "M2_gammarho")
h_s_final  = Data_final.Histo1D(("s_final", "S_{#gammaN} apres coupures; S_{#gammaN} (GeV^{2}); Counts", 50, 0, 25), "s")

h_t_raw      = Data.Histo1D(("t_raw", "-t brut; -t (GeV^{2}); Counts", 100, 0, 2), "mt")
h_tprime_raw = Data.Histo1D(("tprime_raw", "-t' brut; -t' (GeV^{2}); Counts", 100, 0, 6), "mtprime")
h_uprime_raw = Data.Histo1D(("uprime_raw", "-u' brut; -u' (GeV^{2}); Counts", 100, 0, 6), "muprime")
h_cos_final  = Data_final.Histo1D(("cos_hel", "cos#theta_{hel} (#rho#rightarrow#pi^{+}#pi^{-}); cos#theta; Counts", 50, -1, 1), "cosThetaHel")

h_dalitz   = Data_Exlusif.Histo2D(("dalitz", "Dalitz; M^{2}_{p#pi^{+}} (GeV^{2}); M^{2}_{p#pi^{-}} (GeV^{2})", 100, 1, 4, 100, 1, 4), "M2_ppip", "M2_ppim")
h_tp_vs_up = Data_final.Histo2D(("tp_vs_up", "-t' vs -u'; -u' (GeV^{2}); -t' (GeV^{2})", 50, 0, 6,  50, 0, 6),  "muprime", "mtprime")
h_M2_vs_up = Data_final.Histo2D(("M2_vs_up", "M^{2}_{#gamma#rho} vs -u'; -u' (GeV^{2}); M^{2}_{#gamma#rho} (GeV^{2})", 50, 1, 6, 50, 2, 6), "muprime", "M2_gammarho")

h_PvsTheta_prot   = Data.Histo2D(("PvsTheta_prot",   "p : P vs #theta; #theta (rad); P (GeV)",        100, 0, 3.2, 100, 0, 6),   "Theta_prot", "P_prot")
h_ThetavsPhi_prot = Data.Histo2D(("ThetavsPhi_prot", "p : #theta vs #phi; #phi (rad); #theta (rad)",  100, -3.2, 3.2, 100, 0, 3.2), "Phi_prot",   "Theta_prot")
h_PvsTheta_pip    = Data.Histo2D(("PvsTheta_pip",    "#pi^{+} : P vs #theta; #theta (rad); P (GeV)",       100, 0, 3.2, 100, 0, 6),   "Theta_pip", "P_pip")
h_ThetavsPhi_pip  = Data.Histo2D(("ThetavsPhi_pip",  "#pi^{+} : #theta vs #phi; #phi (rad); #theta (rad)", 100, -3.2, 3.2, 100, 0, 3.2), "Phi_pip",   "Theta_pip")
h_PvsTheta_pim    = Data.Histo2D(("PvsTheta_pim",    "#pi^{-} : P vs #theta; #theta (rad); P (GeV)",       100, 0, 3.2, 100, 0, 6),   "Theta_pim", "P_pim")
h_ThetavsPhi_pim  = Data.Histo2D(("ThetavsPhi_pim",  "#pi^{-} : #theta vs #phi; #phi (rad); #theta (rad)", 100, -3.2, 3.2, 100, 0, 3.2), "Phi_pim",   "Theta_pim")
h_PvsTheta_phot   = Data.Histo2D(("PvsTheta_phot",   "#gamma : P vs #theta; #theta (rad); P (GeV)",        100, 0, 3.2, 100, 0, 6),   "Theta_phot", "P_phot")
h_ThetavsPhi_phot = Data.Histo2D(("ThetavsPhi_phot", "#gamma : #theta vs #phi; #phi (rad); #theta (rad)",  100, -3.2, 3.2, 100, 0, 3.2), "Phi_phot",   "Theta_phot")


c_total   = Data.Count()
c_Exlusif = Data_Exlusif.Count()
c_Rho     = Data_Rho.Count()
c_pi      = Data_pi.Count()
c_xi      = Data_xi.Count()
c_final   = Data_final.Count()

n_total   = c_total.GetValue()
n_Exlusif = c_Exlusif.GetValue()
n_Rho     = c_Rho.GetValue()
n_pi      = c_pi.GetValue()
n_xi      = c_xi.GetValue()
n_final   = c_final.GetValue()

print(f"n_total   = {n_total}")
print(f"n_Exlusif = {n_Exlusif}")
print(f"n_Rho     = {n_Rho}")
print(f"n_pi      = {n_pi}")
print(f"n_xi      = {n_xi}")
print(f"n_final   = {n_final}")


# ================= LIGNES DE COUPURES =================
cut_lines = []   # garder une reference pour eviter le GC Python

def _vline(x, ymin, ymax):
    l = ROOT.TLine(x, ymin, x, ymax)
    l.SetLineColor(ROOT.kRed); l.SetLineStyle(2); l.SetLineWidth(2)
    cut_lines.append(l); return l

def _hline(y, xmin, xmax):
    l = ROOT.TLine(xmin, y, xmax, y)
    l.SetLineColor(ROOT.kRed); l.SetLineStyle(2); l.SetLineWidth(2)
    cut_lines.append(l); return l

# --- M(pi+pi-) brut : 0.3 < M_Rho < 1.1 ---
l_rho_lo = _vline(0.3, 0, Histo_Rho.GetMaximum()*1.05)
l_rho_hi = _vline(1.1, 0, Histo_Rho.GetMaximum()*1.05)

# --- Masse manquante : |miss_m_tot| < 0.2  ->  miss_m_tot^2 < 0.04 ---
l_mm = _vline(0.04, 0, h_miss_raw.GetMaximum()*1.05)

# --- Energie manquante : |miss_E_tot| < 0.2 ---
l_dE_lo = _vline(-0.2, 0, h_Emiss_raw.GetMaximum()*1.05)
l_dE_hi = _vline( 0.2, 0, h_Emiss_raw.GetMaximum()*1.05)

# --- xi : 0.04 < xi < 0.33 ---
l_xi_lo = _vline(0.04, 0, h_xi_row.GetMaximum()*1.05)
l_xi_hi = _vline(0.33, 0, h_xi_row.GetMaximum()*1.05)

# --- M2_gammarho > 2 ---
l_M2 = _vline(2.0, 0, h_M2_raw.GetMaximum()*1.05)

# --- -t < 0.5 ---
l_t = _vline(0.5, 0, h_t_raw.GetMaximum()*1.05)

# --- -t' > 1 ---
l_tp = _vline(1.0, 0, h_tprime_raw.GetMaximum()*1.05)

# --- -u' > 1 ---
l_up = _vline(1.0, 0, h_uprime_raw.GetMaximum()*1.05)

# --- coupures pi : mpipprot >= 1.35 (V) et mpimprot <= 1.80 (V) ---
l_mpip = _vline(1.35, 0, h_mpipprot.GetMaximum()*1.05)
l_mpim = _vline(1.80, 0, h_mpimprot.GetMaximum()*1.05)

# --- Dalitz : mpipprot>=1.35 -> M2_ppip>=1.8225 (V) ; mpimprot<=1.80 -> M2_ppim<=3.24 (H) ---
l_dalitz_v = _vline(1.35**2, 1, 4)
l_dalitz_h = _hline(1.80**2, 1, 4)

# --- -t' vs -u' : -u'>1 (V) ; -t'>1 (H) ---
l_tpup_v = _vline(1.0, 0, 6)
l_tpup_h = _hline(1.0, 0, 6)

# --- M2 vs -u' : -u'>1 (V) ; M2>2 (H) ---
l_M2up_v = _vline(1.0, 2, 6)
l_M2up_h = _hline(2.0, 1, 6)


outname = FICHIER.replace(".root", f"_{TREE}_phase1.root")
Data_final.Snapshot("T_final", outname)

# ---------- M(pi+pi-) AVANT coupures : plus de fit ----------
# bin1_high conserve : reutilise par fond_amp_f dans le fit apres coupures
bin1_high = Histo_Rho.FindBin(0.60)

# ---------- M(pi+pi-) APRES coupures : fit (inchange) ----------
fit_left_f, fit_right_f = 0.1, 1.1
bin2_low  = h_rho_final.FindBin(0.65); bin2_high = h_rho_final.FindBin(0.85)
max_bin2  = max(range(bin2_low, bin2_high), key=lambda b: h_rho_final.GetBinContent(b))
amp2_f  = h_rho_final.GetBinContent(max_bin2); mean2_f = h_rho_final.GetBinCenter(max_bin2)
fond_amp_f = h_rho_final.GetBinContent(bin1_high)
f_total_f = ROOT.TF1("f_total_f",
    "[3]*exp(-0.5*((x-[4])/[5])**2) + "
    "[6]*exp([7]*x)",
    fit_left_f, fit_right_f)
f_total_f.SetParameter(3, amp2_f)
f_total_f.SetParameter(4, mean2_f)
f_total_f.SetParameter(5, 0.06)
f_total_f.SetParameter(6, fond_amp_f * np.exp(0.9 * fit_right_f))
f_total_f.SetParameter(7, -0.9)
f_total_f.SetParLimits(4, 0.70, 0.82)
f_total_f.SetParLimits(5, 0.03, 0.15)
h_rho_final.Fit("f_total_f", "RSQ")
Fit_rho_f = f_total_f
mean_rho_f  = f_total_f.GetParameter(4)
sigma_rho_f = f_total_f.GetParameter(5)
print(f"rho (final) : mean = {mean_rho_f:.4f}, sigma = {sigma_rho_f:.4f}")
xmin_f = Fit_rho_f.GetXmin(); xmax_f = Fit_rho_f.GetXmax()
f_gaussRho = ROOT.TF1("f_gaussRho", "[0]*exp(-0.5*((x-[1])/[2])**2)", xmin_f, xmax_f)
f_gaussRho.SetParameters(Fit_rho_f.GetParameter(3), Fit_rho_f.GetParameter(4), Fit_rho_f.GetParameter(5))
f_gaussRho.SetLineColor(ROOT.kBlue)
f_fond_f = ROOT.TF1("f_fond_f", "[0]*exp([1]*x)", xmin_f, xmax_f)
f_fond_f.SetParameters(Fit_rho_f.GetParameter(6), Fit_rho_f.GetParameter(7))
f_fond_f.SetLineColor(ROOT.kGreen + 2)
f_fond_f.SetLineStyle(2)

def make_pull(histo, func, name):
    
    xmin = func.GetXmin(); xmax = func.GetXmax()
    h_pull = histo.Clone(name)
    h_pull.Reset()
    h_pull.SetTitle(f";{histo.GetXaxis().GetTitle()};Pull")
    for i in range(1, histo.GetNbinsX() + 1):
        x = histo.GetBinCenter(i)
        if x < xmin or x > xmax:
            continue
        n_obs = histo.GetBinContent(i)
        n_fit = func.Eval(x)
        if n_fit <= 0:
            continue
        
        h_pull.SetBinContent(i, (n_obs - n_fit) / (n_fit ** 0.5))
        h_pull.SetBinError(i, 0)   # 0 -> juste des points, pas de barres d'erreur
    h_pull.SetMarkerStyle(20)
    h_pull.SetMarkerSize(0.7)
    h_pull.GetYaxis().SetRangeUser(-5, 5)
    return h_pull
    
    _pull_keep = []
def draw_with_pull(canvas, histo, fit_funcs, h_pull):
    canvas.cd()
    x_split = 0.22

    p_top = ROOT.TPad("p_top_" + canvas.GetName(), "", x_split, 0.30, 1, 1.0)
    p_bot = ROOT.TPad("p_bot_" + canvas.GetName(), "", x_split, 0.0,  1, 0.30)
    p_top.SetLeftMargin(0.02); p_bot.SetLeftMargin(0.02)
    p_top.SetBottomMargin(0.02); p_bot.SetTopMargin(0.02); p_bot.SetBottomMargin(0.30)
    p_top.Draw(); p_bot.Draw()
    _pull_keep.extend([p_top, p_bot])

    p_pdist = ROOT.TPad("p_pdist_" + canvas.GetName(), "", 0.0, 0.0, x_split, 0.30)
    p_pdist.SetRightMargin(0.02);  p_pdist.SetLeftMargin(0.30)
    p_pdist.SetTopMargin(0.02);    p_pdist.SetBottomMargin(0.30)
    p_pdist.Draw()
    _pull_keep.append(p_pdist)

    # --- panneau du haut : données + courbes de fit ---
    p_top.cd()
    histo.Draw()
    for f in fit_funcs:
        f.Draw("same")

    # --- panneau du bas : le pull plot (RESTAURÉ) ---
    p_bot.cd()
    h_pull.GetYaxis().SetTitleSize(0.12); h_pull.GetYaxis().SetTitleOffset(0.35)
    h_pull.GetYaxis().SetLabelSize(0.10); h_pull.GetYaxis().SetNdivisions(505)
    h_pull.GetXaxis().SetTitleSize(0.12); h_pull.GetXaxis().SetLabelSize(0.10)
    h_pull.Draw("P")
    xlo = h_pull.GetXaxis().GetXmin(); xhi = h_pull.GetXaxis().GetXmax()
    for y, style in [(0, 1), (2, 2), (-2, 2)]:
        l = ROOT.TLine(xlo, y, xhi, y)
        l.SetLineStyle(style); l.SetLineColor(ROOT.kRed if y == 0 else ROOT.kGray + 1)
        l.Draw("same"); _pull_keep.append(l)

    # --- panneau de gauche : distribution des pulls, axe dirigé vers la gauche ---
    p_pdist.cd()
    PMIN, PMAX = -5.0, 5.0
    h_pdist = ROOT.TH1F("h_pdist_" + canvas.GetName(), "", 25, PMIN, PMAX)
    for i in range(1, h_pull.GetNbinsX() + 1):
        c = h_pull.GetBinContent(i)
        if c == 0:
            continue
        h_pdist.Fill(c)
    npb   = h_pdist.GetNbinsX()
    n_max = h_pdist.GetMaximum()
    if n_max <= 0:
        n_max = 1.0
    x_axis = 1.10 * n_max

    g_pd = ROOT.TGraph()
    g_pd.SetPoint(0, 0.0, PMIN)
    ip = 1
    for j in range(1, npb + 1):
        n    = h_pdist.GetBinContent(j)
        y_lo = h_pdist.GetXaxis().GetBinLowEdge(j)
        y_hi = h_pdist.GetXaxis().GetBinUpEdge(j)
        g_pd.SetPoint(ip, -n, y_lo); ip += 1
        g_pd.SetPoint(ip, -n, y_hi); ip += 1
    g_pd.SetPoint(ip, 0.0, PMAX)

    frame = p_pdist.DrawFrame(-x_axis, PMIN, 0.0, PMAX)
    frame.GetYaxis().SetTitle("Pull")
    frame.GetYaxis().SetTitleSize(0.12); frame.GetYaxis().SetTitleOffset(0.55)
    frame.GetYaxis().SetLabelSize(0.10); frame.GetYaxis().SetNdivisions(505)
    frame.GetXaxis().SetLabelSize(0)
    frame.GetXaxis().SetTitleSize(0)
    frame.GetXaxis().SetTickLength(0)

    g_pd.SetFillColorAlpha(ROOT.kAzure + 1, 0.5)
    g_pd.SetLineColor(ROOT.kAzure + 2); g_pd.SetLineWidth(1)
    g_pd.Draw("F same"); g_pd.Draw("L same")

    gax = ROOT.TGaxis(-x_axis, PMIN, 0.0, PMIN,
                      x_axis, 0.0,
                      503, "")
    gax.SetTitle("N")
    gax.SetLabelSize(0.09); gax.SetTitleSize(0.10)
    gax.SetLabelFont(42);   gax.SetTitleFont(42)
    gax.Draw()
    _pull_keep.extend([h_pdist, frame, g_pd, gax])

    for y, style in [(0, 1), (2, 2), (-2, 2)]:
        lp = ROOT.TLine(-x_axis, y, 0.0, y)
        lp.SetLineStyle(style); lp.SetLineColor(ROOT.kRed if y == 0 else ROOT.kGray + 1)
        lp.Draw("same"); _pull_keep.append(lp)

    canvas.Update()
    
    h_pull_rho_final = make_pull(h_rho_final, f_total_f, "pull_h_rho_final")
c_pull2 = ROOT.TCanvas(f"c_pull_rho_final", "rho_final + pull", 800, 800)
draw_with_pull(c_pull2, h_rho_final, [f_total_f, f_gaussRho, f_fond_f], h_pull_rho_final)
c_pull2.Write(); h_pull_rho_final.Write()
