import pandas as pd
from pathlib import Path
import json

script_dir = Path(__file__).resolve().parent
data_dir = script_dir.parent / "data"
input_file = data_dir / "RegisterReference.csv"
output_file = data_dir / "Registers.json"

print(f"Input File: {input_file}")
print(f"Output File: {output_file}")

df = pd.read_csv(input_file)

result = []

df = pd.DataFrame(df)

print()
print(f"DataFrame dimensions: {df.shape}")
print(df.head())
print()

# Group by both 'Address' and 'Symbol' columns and merge rows with the same address and symbol
def merge_values(series):
    """
    Merges unique values from a list-like series into a single comma-separated string.

    Args:
        series (iterable): An iterable containing elements to be merged.

    Returns:
        str: A comma-separated string of unique elements from the input series.
    """
    seen = set()
    result = []
    for item in series:
        if item not in seen:
            seen.add(item)
            result.append(item)
    return ','.join(result)

merged_df = df.groupby(['Address', 'Symbol']).agg(lambda x: merge_values(x.astype(str))).reset_index()

print()
print(f"DataFrame dimensions: {merged_df.shape}")
print(merged_df.head())
print()

# Convert the merged DataFrame to a dictionary
merged_dict = merged_df.to_dict(orient='records')

# Write the dictionary to a JSON file
with open(output_file, 'w') as f:
    json.dump(merged_dict, f, indent=4)

print(f"Merged data has been written to {output_file}")