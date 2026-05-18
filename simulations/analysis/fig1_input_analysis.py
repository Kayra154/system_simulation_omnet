import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('Agg')
from scipy import stats

plt.rcParams.update({'font.family': 'serif', 'font.size': 9, 'savefig.dpi': 300})

LAMBDA = 0.00175

df = pd.read_csv('../vectors_variates.csv')
vec_rows = df[df['name'] == 'emergencyVariate:vector']
print(f"Found {len(vec_rows)} vector rows")

all_variates = []
for _, row in vec_rows.iterrows():
    if pd.notna(row.get('vecvalue')) and row['vecvalue']:
        all_variates.extend(list(map(float, row['vecvalue'].split())))

v = np.array(all_variates)
print(f"Total variates: {len(v)}")
print(f"Mean: {v.mean():.2f} (expected 1/λ = {1/LAMBDA:.2f})")
print(f"Min: {v.min():.4f}, Max: {v.max():.4f}")
print(f"Negatives: {(v<0).sum()}")

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(7.16, 2.8))

ax1.hist(v, bins=60, density=True, alpha=0.6, color='steelblue',
         edgecolor='black', linewidth=0.3, label='Empirical')
x = np.linspace(0, np.percentile(v, 99.5), 300)
ax1.plot(x, LAMBDA * np.exp(-LAMBDA * x), 'r-', lw=1.5, label=f'Exp(λ={LAMBDA})')
ax1.set_xlabel('Variate value')
ax1.set_ylabel('Probability density')
ax1.set_title('(a) Histogram vs Theoretical PDF')
ax1.legend(loc='upper right')
ax1.grid(alpha=0.3)

stats.probplot(v, dist=stats.expon, sparams=(0, 1/LAMBDA), plot=ax2)
ax2.set_title('(b) Q-Q Plot')
ax2.grid(alpha=0.3)

plt.tight_layout()
plt.savefig('../figures/fig1_input_analysis.pdf')
plt.savefig('../figures/fig1_input_analysis.png')

ks_stat, ks_p = stats.kstest(v, 'expon', args=(0, 1/LAMBDA))
print(f"\nKS test: statistic={ks_stat:.4f}, p-value={ks_p:.6f}")
print(f"  (p > 0.05 → exponential not rejected)")
print(f"\nSaved: figures/fig1_input_analysis.pdf")
