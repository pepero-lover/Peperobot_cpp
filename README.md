# Peperobot_Cpp

기존 Peperobot_chess 자바 버전에서 스톡피쉬 nnue 평가함수를 탑재한 엔진입니다.

⚠️ 개인 학습/취미 프로젝트입니다. 상업적 목적이 아닌 Lichess-bot 운용등을 목표로 개발중입니다.

## NNUE

이 프로젝트의 NNUE 평가 코드는 [Stockfish 16버전](https://github.com/official-stockfish/Stockfish/tree/sf_16)의 feature transformer / layer 구현(HalfKAv2_hm, `AffineTransformSparseInput`, `ClippedReLU` 등)을 이식하여 사용합니다. Accumulator 스택 관리와 incremental update 로직(king-move 기준 refresh/update 분기)은 이 프로젝트에서 직접 구현했습니다.


그리고 nnue 는 스톡피쉬 16 버전의 **[nn-5af11540bbfe.nnue](https://github.com/official-stockfish/networks/blob/master/nn-5af11540bbfe.nnue)** 를 사용하였습니다.


## License

이 프로젝트는 [GNU General Public License v3.0](LICENSE) (또는 이후 버전)을 따릅니다.

NNUE 평가 코드 일부가 GPLv3로 배포되는 Stockfish의 코드를 기반으로 하기 때문에, 이 저장소 전체가 GPLv3 조건의 적용을 받습니다. 이 프로젝트를 수정하여 배포(lichess bot 운영 포함)할 경우, 소스코드 공개 등 GPLv3 조건을 준수해야 합니다.

## Acknowledgements

- [Stockfish](https://github.com/official-stockfish/Stockfish) — NNUE 평가 구현 참고/이식