# Alibaba Streaming v2 patch

Adds:
- progress output every configurable number of source rows
- rows/sec and ETA for row-bounded pilots
- atomic checkpoint CSV/JSON snapshots
- partial-result saving on Ctrl+C

Apply from the root of the existing `ftl_step01` project:

```bash
unzip -o ~/Downloads/ftl_alibaba_streaming_v2_patch.zip
python3 -m unittest discover -s tests -v
```

Recommended pilot:

```bash
aws s3 cp \
  s3://cache-datasets/cache_dataset_txt/2020_alibabaBlock/alibabaBlock2020.csv.zst \
  - --no-sign-request |
zstd -dc |
python3 -m simulator.run_analyze_alibaba \
  --input - \
  --output-dir results/alibaba_pilot_10m \
  --max-source-rows 10000000 \
  --minimum-candidate-requests 2000 \
  --progress-every-rows 1000000 \
  --checkpoint-every-rows 5000000
```
