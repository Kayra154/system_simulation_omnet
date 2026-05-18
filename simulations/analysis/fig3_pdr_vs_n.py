import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('Agg')

plt.rcParams.update({'font.family': 'serif', 'font.size': 9, 'savefig.dpi': 300})

df = pd.read_csv('../all_scalars.csv')
df['configname'] = df['run'].str.split('-').str[0]
df['value'] = pd.to_numeric(df['value'], errors='coerce')

N_MAP = {'N5': 5, 'N10': 10, 'N20': 20, 'N50': 50, 'N75': 75}

print(f"{'Config':6s} {'N':>3s} | {'Urgent PDR (%)':>20s} | {'Normal PDR (%)':>20s}")
print("-" * 70)

results = []
for cfg, n in N_MAP.items():
   
    u_sent_mask = (df['configname']==cfg) & (df['name']=='urgentPacketsSentTotal')
    n_sent_mask = (df['configname']==cfg) & (df['name']=='normalPacketsSentTotal')
    u_sent = df.loc[u_sent_mask].groupby('run')['value'].sum()
    n_sent = df.loc[n_sent_mask].groupby('run')['value'].sum()
    
    
    u_recv = df[(df['configname']==cfg) & (df['name']=='urgentReceivedTotal')].set_index('run')['value']
    n_recv = df[(df['configname']==cfg) & (df['name']=='normalReceivedTotal')].set_index('run')['value']
    
    
    u_pdr = (u_recv / u_sent.reindex(u_recv.index).replace(0, np.nan)).dropna() * 100
    n_pdr = (n_recv / n_sent.reindex(n_recv.index).replace(0, np.nan)).dropna() * 100
    
    results.append({
        'N': n,
        'u_mean': u_pdr.mean(), 'u_ci': 1.96*u_pdr.std()/np.sqrt(len(u_pdr)),
        'n_mean': n_pdr.mean(), 'n_ci': 1.96*n_pdr.std()/np.sqrt(len(n_pdr)),
    })
    print(f"{cfg:6s} {n:>3d} | {u_pdr.mean():>8.2f} ± {u_pdr.std():>5.2f}     | "
          f"{n_pdr.mean():>8.2f} ± {n_pdr.std():>5.2f}")

R = pd.DataFrame(results)
fig, ax = plt.subplots(figsize=(3.5, 2.8))
ax.errorbar(R['N'], R['u_mean'], yerr=R['u_ci'], fmt='o-', color='crimson',
            capsize=4, lw=1.5, ms=5, label='Urgent')
ax.errorbar(R['N'], R['n_mean'], yerr=R['n_ci'], fmt='s-', color='steelblue',
            capsize=4, lw=1.5, ms=5, label='Normal')

ax.set_xlabel('Number of patients (N)')
ax.set_ylabel('Packet Delivery Ratio (%)')
ax.set_xticks(R['N'])
ax.axhline(99, color='gray', ls=':', lw=1)
ax.text(R['N'].iloc[0], 99.5, '99% target', fontsize=7, color='gray')
ax.legend(loc='lower left')
ax.grid(alpha=0.3)
ax.set_ylim([0, 105])

plt.tight_layout()
plt.savefig('../figures/fig3_pdr_vs_n.pdf')
plt.savefig('../figures/fig3_pdr_vs_n.png')
print(f"\nSaved: figures/fig3_pdr_vs_n.pdf")
