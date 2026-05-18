
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('Agg')

plt.rcParams.update({'font.family': 'serif', 'font.size': 9, 'savefig.dpi': 300})

df = pd.read_csv('../vectors_baseline.csv')

qlen = df[(df['name'] == 'queueLength:vector') & 
          (df['module'].str.contains('HospitalServer.app'))]
print(f"Total entries: {len(qlen)}, unique runs: {qlen['run'].nunique()}")


run0_id = qlen['run'].iloc[0]
run0_data = qlen[qlen['run'] == run0_id]
print(f"Run 0 ({run0_id}): {len(run0_data)} CSV chunks")

all_pairs = set()
for _, r in run0_data.iterrows():
    if pd.notna(r['vectime']) and r['vectime']:
        t_arr = list(map(float, r['vectime'].split()))
        v_arr = list(map(float, r['vecvalue'].split()))
        all_pairs.update(zip(t_arr, v_arr))

sorted_pairs = sorted(all_pairs)
times = np.array([p[0] for p in sorted_pairs])
values = np.array([p[1] for p in sorted_pairs])
print(f"Unique (t,v) pairs: {len(times)}, range {times.min():.1f}-{times.max():.1f}s")
print(f"Queue min={values.min()}, max={values.max()}")


mask = times >= 60
t_post = times[mask]
v_post = values[mask]
durations = np.diff(t_post)
time_avg = np.sum(v_post[:-1] * durations) / (t_post[-1] - t_post[0])
print(f"Time-weighted mean (post-warmup): {time_avg:.4f}")

fig, ax = plt.subplots(figsize=(7.16, 2.8))
ax.plot(times/60, values, color='steelblue', lw=0.6, alpha=0.8, drawstyle='steps-post')
ax.axvspan(0, 1, alpha=0.15, color='red', label='Warmup (60s)')
ax.axvline(1, color='red', ls='--', lw=1)
ax.set_xlabel('Simulation time (minutes)')
ax.set_ylabel('Queue length (packets)')
ax.set_title(f'Hospital server queue evolution — N=20 (time-avg = {time_avg:.4f})')
ax.legend(loc='upper right')
ax.grid(alpha=0.3)
ax.set_ylim([-0.1, 1.5])

plt.tight_layout()
plt.savefig('../figures/fig4_queue_time.pdf')
plt.savefig('../figures/fig4_queue_time.png')
print(f"\nSaved: figures/fig4_queue_time.pdf")
