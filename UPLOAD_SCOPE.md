# GitHub 보관 범위와 재현 방법

이 폴더는 `01_Online_HotCold_FTL_Final.zip`의 연구 코드·테스트·실험 근거를
분리한 복사본입니다. 원본 아카이브와 개인 학습·면접 자료는 로컬에 별도로 보존합니다.

- `simulator/`: FTL, GC, analytical latency, Alibaba 변환·스트리밍·추출 구현
- `tests/`: 원래 단위 테스트
- `samples/`: 작게 실행할 수 있는 입력 예제
- `data/`, `figures/`: 정책 비교표와 최종 그림
- `reports/`: 최종 보고서와 결과 요약 workbook
- `results_archive.zip`: 원래 `results/` 구조를 유지한 실험 근거. MSR·Alibaba
  비교 결과와 Alibaba dev4·49·740의 100,000-request 표준 입력이 포함됩니다.

실행 전 이 폴더에서 결과 패키지를 풉니다.

```sh
python3 -m zipfile -e results_archive.zip .
python3 -m unittest discover -s tests -v
python3 -m simulator.run_compare_hot_cold --help
```

그 다음 `README.md`의 dev4 비교 명령을 실행하면 됩니다. 재실행 결과는
기존 결과와 섞지 않도록 새 output directory에 기록합니다.

Alibaba 전체 trace는 GitHub에 올리지 않았습니다. 당시 사용한 입력 위치와
스트리밍 방식은 `ALIBABA_STREAMING_V2.md`에 남겨 두었습니다. 이 문서의
과거 patch 적용 안내는 역사적 기록이며, 이 export에는 구현이 이미 포함되어 있습니다.
전체 trace를 새로 받거나 재배포할 때는 원 데이터 제공 조건을 확인합니다.

Latency는 실제 SSD 장치 측정값이 아닌 동일한 analytical model 안의 상대 비교입니다.
