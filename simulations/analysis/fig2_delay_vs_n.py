
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('Agg')

plt.rcParams.update({'font.family': 'serif', 'font.size': 9, 'savefig.dpi': 300})

df = pd.read_csv('../all_scalars.csv')
df['configname'] = df['run'].str.split('-').str[0]

N_MAP = {'N5': 5, 'N10': 10, 'N20': 20, 'N50': 50, 'N75': 75}

def get_mean_ci(scenario, metric):
    mask = (df['configname'] == scenario) & (df['name'] == metric)
    vals = df.loc[mask, 'value'].dropna()
    vals = pd.to_numeric(vals, errors='coerce').dropna()
    if len(vals) == 0:
        return np.nan, np.nan, 0
    return vals.mean(), 1.96 * vals.std() / np.sqrt(len(vals)), len(vals)

print(f"{'Config':6s} {'N':>3s} | {'Urgent E2E (ms)':>22s} | {'Normal E2E (ms)':>22s}")
print("-" * 70)

urg_m, urg_c, nrm_m, nrm_c, Ns = [], [], [], [], []
for cfg, n in N_MAP.items():
    u, uc, nu = get_mean_ci(cfg, 'urgentEndToEndDelay:mean')
    nm, nc, nn = get_mean_ci(cfg, 'normalEndToEndDelay:mean')
    urg_m.append(u*1000); urg_c.append(uc*1000)
    nrm_m.append(nm*1000); nrm_c.append(nc*1000)
    Ns.append(n)
    print(f"{cfg:6s} {n:>3d} | {u*1000:>10.2f} ± {uc*1000:>6.2f} (n={nu:2d}) | "
          f"{nm*1000:>10.2f} ± {nc*1000:>6.2f} (n={nn:2d})")

fig, ax = plt.subplots(figsize=(3.5, 2.8))
ax.errorbar(Ns, urg_m, yerr=urg_c, fmt='o-', color='crimson',
            capsize=4, lw=1.5, ms=5, label='Urgent')
ax.errorbar(Ns, nrm_m, yerr=nrm_c, fmt='s-', color='steelblue',
            capsize=4, lw=1.5, ms=5, label='Normal')


ax.axhline(1700, color='gray', ls=':', lw=1, alpha=0.7)
ax.text(Ns[-1], 1680, 'SF=12 airtime ≈ 1.7s', fontsize=7, color='gray',
        ha='right', va='top')

ax.set_xlabel('Number of patients (N)')
ax.set_ylabel('Mean E2E delay (ms)')
ax.set_xticks(Ns)
ax.set_ylim([0, 3000])
ax.legend(loc='upper left')
ax.grid(alpha=0.3)

plt.tight_layout()
plt.savefig('../figures/fig2_delay_vs_n.pdf')
plt.savefig('../figures/fig2_delay_vs_n.png')
print(f"\nSaved: figures/fig2_delay_vs_n.pdf")
