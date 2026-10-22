# ATLAS-SSD blockers

- Missing: previous dedicated ATLAS checkpoint/C++ work artifact, if one exists outside the sources searched. Proceed on the verified existing research branch; user agreed to state recovery.
- Missing MSR historical input; do not fabricate or replace it and call it the original.
- MQSim modified engine source and dynamic-lending path instrumentation unavailable.
- GitHub connector returns 404 for accessible private repos; authenticated CLI reads succeed with network access. User explicitly approved this private repo/branch/payload after the initial automatic-review rejection; implementation 400ec1e pushed successfully.
- Catch2 v2.13.10 vendored with original BSL-1.0 license and file hashes; clang-format 20.1.8 installed in /private/tmp/atlas-validation-tools-20261022. clang-tidy unavailable; actual Clang static analyzer executed instead. Temporary formatting tool may need reinstall next session.
- Linux GCC + Clang sanitizer CI passed on source 400ec1e. New novelty audit, untouched holdout, MQSim/external validation and independent red-team review remain unperformed.
