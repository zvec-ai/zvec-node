import json

main_db = "build/compile_commands.json"
zvec_db = "build/zvec-build/compile_commands.json"

# Load both files
with open(main_db) as f:
    main = json.load(f)

with open(zvec_db) as f:
    zvec = json.load(f)

# Merge
merged = main + zvec

# Overwrite the original main compile_commands.json
with open(main_db, "w") as f:
    json.dump(merged, f, indent=2)

print(f"Merged {len(merged)} entries into {main_db}")
