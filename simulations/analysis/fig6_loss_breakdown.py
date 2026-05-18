
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import sys
matplotlib.use('Agg')

plt.rcParams.update({'font.family': 'serif', 'font.size': 9, 'savefig.dpi': 300})


df = pd.read_csv('../all_scalars.csv')


if 'configname' not in df.columns:
    
    if 'attrname' in df.columns:
        config_map = df[(df['type'] == 'runattr') & (df['attrname'] == 'configname')][['run', 'attrvalue']]
        if not config_map.empty:
            config_map = config_map.rename(columns={'attrvalue': 'configname'})
            df = df.merge(config_map, on='run', how='left')
    
    
    if 'configname' not in df.columns and 'run' in df.columns:
        df['configname'] = df['run'].astype(str).str.split('-').str[0]


if 'configname' not in df.columns:
    print(f"Error: 'configname' column could not be found or generated.", file=sys.stderr)
    print(f"Available columns in your CSV are: {list(df.columns)}", file=sys.stderr)
    print("Please check your CSV structure.", file=sys.stderr)
    sys.exit(1)


N_MAP = {'N5': 5, 'N10': 10, 'N20': 20, 'N50': 50, 'N75': 75}

def sum_metric(cfg, metric):
    mask = (df['configname']==cfg) & (df['name']==metric)
    return df.loc[mask].groupby('run')['value'].sum().mean()

stats_data = []
for cfg, n in N_MAP.items():
    u_sent = sum_metric(cfg, 'urgentPacketsSentTotal')
    n_sent = sum_metric(cfg, 'normalPacketsSentTotal')
    u_recv = sum_metric(cfg, 'urgentReceivedTotal')
    n_recv = sum_metric(cfg, 'normalReceivedTotal')
    u_drop = sum_metric(cfg, 'urgentDroppedTotal')
    n_drop = sum_metric(cfg, 'normalDroppedTotal')
    
    total = u_sent + n_sent
    radio_loss = total - (u_recv + n_recv)
    queue_drop = u_drop + n_drop
    delivered = (u_recv + n_recv) - queue_drop
    
    stats_data.append({
        'N': n,
        'delivered': 100 * delivered / total if total > 0 else 0,
        'radio': 100 * radio_loss / total if total > 0 else 0,
        'queue': 100 * queue_drop / total if total > 0 else 0,
    })

S = pd.DataFrame(stats_data)
x = np.arange(len(S))
fig, ax = plt.subplots(figsize=(3.5, 2.8))
ax.bar(x, S['delivered'], color='seagreen', label='Delivered')
ax.bar(x, S['radio'], bottom=S['delivered'], color='orange', label='Radio loss')
ax.bar(x, S['queue'], bottom=S['delivered']+S['radio'], color='crimson', label='Queue drop')

ax.set_xticks(x)
ax.set_xticklabels([f"N={n}" for n in S['N']])
ax.set_ylabel('Percentage of generated packets (%)')
ax.legend(loc='lower left', fontsize=7)
ax.set_ylim([0, 105])
ax.grid(alpha=0.3, axis='y')
plt.tight_layout()
plt.savefig('../figures/fig6_loss_breakdown.pdf')
