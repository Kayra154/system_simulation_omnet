
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('Agg')

plt.rcParams.update({'font.family': 'serif', 'font.size': 9, 'savefig.dpi': 300})

df = pd.read_csv('../vectors_baseline.csv')

def collect(name):
    rows = df[df['name'] == name]
    vals = []
    for _, r in rows.iterrows():
        if pd.notna(r.get('vecvalue')) and r['vecvalue']:
            vals.extend(list(map(float, r['vecvalue'].split())))
    return np.array(vals)

u = collect('urgentWaitTime:vector') * 1000
n = collect('normalWaitTime:vector') * 1000

print(f"Urgent: n={len(u)}, mean={u.mean():.2f}ms, max={u.max():.2f}ms")
print(f"Normal: n={len(n)}, mean={n.mean():.2f}ms, max={n.max():.2f}ms")

bins = np.linspace(0, np.percentile(np.concatenate([u,n]), 99), 50)

fig, ax = plt.subplots(figsize=(3.5, 2.8))
ax.hist(u, bins=bins, alpha=0.6, color='crimson', edgecolor='darkred', linewidth=0.3,
        label=f'Urgent (mean={u.mean():.1f}ms)')
ax.hist(n, bins=bins, alpha=0.6, color='steelblue', edgecolor='navy', linewidth=0.3,
        label=f'Normal (mean={n.mean():.1f}ms)')
ax.set_xlabel('Wait time in queue (ms)')
ax.set_ylabel('Frequency')
ax.set_yscale('log')
ax.legend(loc='upper right')
ax.grid(alpha=0.3, which='both')
plt.tight_layout()
plt.savefig('../figures/fig5_wait_histogram.pdf')
