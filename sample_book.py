"""
Sample positions from Andrew Grant's E12.52 .book file.
Format: FEN [result] eval_score
We extract: FEN [result]
"""
import random
import sys

input_file = r"F:\Polarity\E12.52-1M-D12-Resolved\E12.52-1M-D12-Resolved.book"
output_file = r"F:\Polarity\training_positions.txt"
target = 500000

print(f"Counting lines in {input_file}...")

# First pass: count lines
total = 0
with open(input_file, 'r') as f:
    for _ in f:
        total += 1

print(f"Total positions: {total}")

# Calculate sampling rate
sample_rate = target / total
print(f"Sampling {target} positions (rate: {sample_rate:.4f})")

# Second pass: reservoir sampling
random.seed(42)
sampled = []

with open(input_file, 'r') as f:
    for i, line in enumerate(f):
        if random.random() < sample_rate * 1.05:  # slight oversample then truncate
            # Parse: FEN [result] eval
            # We need to keep just "FEN [result]"
            bracket_start = line.find('[')
            bracket_end = line.find(']')
            if bracket_start < 0 or bracket_end < 0:
                continue
            # Keep FEN + [result]
            entry = line[:bracket_end + 1].strip()
            sampled.append(entry)

        if (i + 1) % 1000000 == 0:
            print(f"  Processed {i+1}/{total}, collected {len(sampled)}")

# Shuffle and truncate to exact target
random.shuffle(sampled)
sampled = sampled[:target]

with open(output_file, 'w') as f:
    for entry in sampled:
        f.write(entry + '\n')

print(f"\nDone! Wrote {len(sampled)} positions to {output_file}")

# Stats
results = {"1.0": 0, "0.5": 0, "0.0": 0}
for entry in sampled:
    if "[1.0]" in entry:
        results["1.0"] += 1
    elif "[0.0]" in entry:
        results["0.0"] += 1
    elif "[0.5]" in entry:
        results["0.5"] += 1

print(f"White wins: {results['1.0']} ({100*results['1.0']/len(sampled):.1f}%)")
print(f"Draws:      {results['0.5']} ({100*results['0.5']/len(sampled):.1f}%)")
print(f"Black wins: {results['0.0']} ({100*results['0.0']/len(sampled):.1f}%)")
