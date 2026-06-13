# Polarity — Legacy History & Development Archive

This document preserves the original development history of Polarity before the commit history was reorganized into a versioned scheme. It serves as a record of the raw, unedited journey of building this engine from scratch.

---

## Why This Reorganization Was Necessary

When I started Polarity in May 2025, I wrote commit messages the way most solo developers do, quick, sometimes messy, sometimes a single line dashed off at 2 AM after finally getting transposition tables to stop crashing. Messages like "MAde some more changes to Transposition Tables" or "Random backup commit, will come back to this later" told the story honestly, but they didn't tell it *clearly*.

By the time V2 arrived in June 2026, the engine had grown from a simple bitboard skeleton to a full-featured chess engine with Texel tuning, king safety zones, and dozens of pruning techniques. The problem was that the commit history had become unreadable. Version numbers in commit messages didn't match any consistent scheme (V6, V7, V8, V9, V10 were all used internally but jumbled in the log), scrapped experiments sat alongside production code, and there was no way to tell at a glance what era a commit belonged to.

More importantly, V2 introduced AI-assisted development. This was a significant shift in how the engine was built, and I wanted to be transparent about it. The old commit history made no distinction between human-written and AI-assisted code. Reorganizing everything into a clear V0/V1/V2 scheme made it possible to draw that line cleanly:

- **V0.x and V1.x** — Every line of code, every bug, every fix, every late-night debugging session was done entirely by hand. No AI, no copilots, no suggestions. Just me, the Chess Programming Wiki, and a lot of trial and error.
- **V2.x** — Changes were made with AI assistance. The ideas, direction, and decisions were still mine, but the implementation had a collaborator(and quite a fast one).

The reorganization rewrote commit messages (not code) to use a consistent `Vx.y.z` prefix, making it possible to trace the engine's evolution at a glance. All original author dates were preserved and the timeline is authentic, only the labels have changed.

---

## Original README (Pre-Reorganization)

Below is the complete, unedited README as it existed before the V0/V1/V2 restructuring. This includes the old version numbering (V1 through V14), the scrapped experiments, and all the original wording.

---

### Version History (Original)

#### Polarity V14

- Added per-side attack bitboards for threat detection and safe mobility.
- Mobility now excludes squares defended by enemy pawns (safe mobility).
- Added threat evaluation: minor pieces threatening rooks/queens, hanging pieces, pawn push threats.
- Added space evaluation for central territory control in middlegame.
- Added pawn storm bonus for pawns advancing on enemy king files.
- Fixed knight outpost logic — now requires pawn support and can't be kicked by enemy pawns.
- Added center control, connected rooks, pawn duo, and king adjacent open file bonuses.
- +42 Elo points than V13.

#### Polarity V13

- Halved free passer bonus for rook pawns (a/h file) since they're harder to promote.
- Added free passer bonus for passed pawns with no blocker in front.
- Improved mop-up evaluation with stronger edge-pushing weights.
- Added endgame scaling for drawish positions (opposite color bishops, etc.).
- +15 Elo points than V12.

#### Polarity V12

- Never prune the TT/hash move in LMP/futility — it's almost always the best move.
- Extended reverse futility pruning up to depth 6 with tighter margins.
- Added countermove history for better quiet move ordering.
- Improved aspiration window scaling with gradual widening (1.5x growth instead of fixed steps).
- +30 Elo points than V11.

#### Polarity V11

- Retuned all eval parameters on a 500k position dataset using the Texel tuner.
- Added Texel tuner (Adam optimization + local search) for automated parameter tuning.
- Added bad bishop penalty and blocked passer detection.
- Added king attack zone evaluation with weighted piece attacks and safety table.
- Fixed duplicate evaluation component counting.
- +56 Elo points than V10.

#### Polarity V10

- Fixed MATEVALUE bug in 50 move rule detection.
- Incremented depth above quiescence search.
- +35 Elo points than V9.

#### Polarity V9

- Added Static Evaluation Pruning (V9a - +70 Elo points than V8).
- Added Razoring in Search (V9b - +80 Elo points than V8).
- Allowed checks to be processed in Quiescence Search (V9c - +100 Elo points than V8).
- +100 Elo points than V8.

#### Polarity V8

- Added Minor Piece Endgame Helper for Bishops.
- +50 Elo points than V7.

#### Polarity V10 (scraped)

- Added Checks in Move Ordering.
- Perform Worse than V7.
- Reverted to V8, with original mobility bonus for minor pieces in endgame.

#### Polarity V9 (scraped)

- Allowed Checks to be processed in Quiescence Search.
- Added back Mobility Bonus for minor pieces in endgame with lesser weights.
- Performance not stable, sometimes wins against V7 sometimes loses.

#### Polarity V8 (scraped)

- Added Minor Piece Endgame Helper for Bishops.
- Removed mobility bonus for minor pieces in endgame.
- -40 Elo points than V7.

#### Polarity V7

- Added Insufficient Material Detection.
- +30 Elo points than V6.

#### Polarity V6

- Added Mobility Bonus for all pieces to encourage piece activity.
- Added King Shield Bonus
- Transposition Table now clears only on UCI new game command.
- +70 Elo points than V5.

#### Polarity V5

- Added Best Move history and ordering in Transposition Table.
- +40 Elo points than V4.

#### Polarity V4

- Added Tapered evaluation.
- MopUp evaluation for endgame checkmates.
- +50 Elo points than V3.

#### Polarity V3

- Added PeSTO evaluation tables.
- +80 Elo points than V2.

#### Polarity V2

- Added Passed Pawn bonus and Isolated Pawn penalty.
- +30 Elo points than V1.

#### Polarity V1

- Initial release with basic evaluation and search.
- Evaluation based on estimated piece values and piece-square tables.

---

## Version Mapping: Old to New

For reference, here is how the original internal version numbers map to the new scheme:


| Old Name       | New Version | Era                   |
| -------------- | ----------- | --------------------- |
| (initial)      | V0.1 – V0.5 | Foundation            |
| V1 (original)  | V1.1        | UCI + basic search    |
| V2 (original)  | V1.9        | PeSTO eval            |
| V3 (original)  | V1.9        | PeSTO eval            |
| V4 (original)  | V1.9.1      | Mop-up eval           |
| V5 (original)  | V1.10       | Dynamic TT            |
| V6 (original)  | V1.11       | Mobility              |
| V7 (original)  | V1.12       | Insufficient material |
| V8 (original)  | V1.13       | Minor piece endgame   |
| V9 (original)  | V1.14       | Static eval pruning   |
| V10 (original) | V1.15       | MATEVALUE fix         |
| V11 (original) | V2.7 – V2.8 | Texel tuner           |
| V12 (original) | V2.9        | Search improvements   |
| V13 (original) | V2.10       | Endgame eval          |
| V14 (original) | V2.11       | Structural eval       |


---

## Original Commit Messages (Complete Log)

These are the original, unedited commit messages as they appeared before the rewrite:

```
c38acee  2025-05-25  Initial commit
bf0f9e8  2025-05-25  Basic Skeleton Done & Leaper Pieces Attack Tables working.
5791918  2025-05-25  Slider pieces working. Some tests remaining
b68a0ed  2025-05-25  Sliding Pieces working. Basic tests Done
eb48857  2025-05-26  Pawn Moves Working.
e02aa9e  2025-05-26  Make Move, take back move and legal move detection in place. PERFT testing required.
8752ca3  2025-05-26  Perft Tests passed.
1eabdd7  2025-05-26  Updated Readme and Makefile
7b05109  2025-05-27  Added basic UCI support.
027b088  2025-05-27  Changed files to allow Board as pointer everywhere.
c1bea53  2025-05-27  Negamax and Evaluation basics done.
882d633  2025-05-27  made some changes to fix UCI
07bddfe  2025-05-27  Move Ordering (Captures for now) done.
e8a8254  2025-05-27  Principal Variation Implementation along with iterative deepening
a16a668  2025-05-27  Added LMR to the file. Fixed PV duplication.
670aa52  2025-05-28  Fixed some formatting
24e8031  2025-05-28  Aspiration Windows added.
9e16ddb  2025-05-29  Added transposition tables! Fixed search bugs..
6e60780  2025-05-29  MAde some more changes to Transposition Tables
dc88ae2  2025-05-29  Changed move generation order. Decreased searched nodes by around 5%.
d309783  2025-05-29  On main: Repetition Tables messed up everything
d675582  2025-05-29  Intermediate commit - Detecting Repetitions
fb10fc3  2025-05-30  Detecting Repetitions, UCI timing controls added, fixed some search bugs. I think I am done with search for now.
15bc1ae  2025-05-31  PesTO Evaluation + Pawn Analysis
049c6b6  2025-05-31  PeSTO evaluation fix, MopUp Evaluation added...
0a455fa  2025-06-01  File Analysis for Rooks and Kings added.
1841d07  2025-06-01  Fixed Move generation Bug at invalid b2-a1 pawn captures, Added Fifty Move Draws, Testing remains
b96009c  2025-06-02  Changed data type of nodes searched
1e6cdbb  2025-06-25  made code consistent for pawn captures to avoid any bugs, changed Readme
66bfeef  2025-06-25  Update README.md
17a2a1f  2025-06-28  Added Dynamic Table Sizing, stores best move as well
531ddb6  2025-06-28  Merge branch 'main' of https://github.com/AkashGupta-26/Polarity
91cc449  2025-06-29  Mobility Bonus, Transposition No clear in a game Final files V6 and V6b
6fa3333  2025-06-30  basic insufficient material detection
f05761f  2025-06-30  Fixed square color detection for bishops in Insufficient Material Files - V7
a08d94f  2025-07-01  Added Minor Piece Endgame Helper, allowed checks to be processed in quiscience search
9f9741b  2025-07-01  Version History Updated Latest Version: V9
1bd329f  2025-07-01  Added Checks in Move Ordering, performs worse somehow
f5d5142  2025-07-01  Reverted to V9
c3e9859  2025-07-02  Random backup commit, will come back to this later
e6c869e  2025-07-03  added static evaluation pruning, razoring and checks in quiescence search. V9 result
9986858  2025-07-03  Fixed MATEVALUE in 50-move rule detection and incremented depth before calling q-search. V10
9a97851  2025-07-03  Merge branch 'Version8', Last version V10 working well
2157d5b  2025-07-20  Fixed double depth increment for checks. V10
fd54bf0  2025-09-28  Added static flags in makefile to allow executable portability
707b86f  2026-06-09  Fixed Search bugs, added search prunings
94d9624  2026-06-09  Add parallel UCI match runner for engine testing, and makefile/gitignore updates for built binaries.
cdf3ccf  2026-06-09  Fix bishop square-color test in minor-piece endgame eval.
a2b90ba  2026-06-09  Fix transposition table lookup, storage, and move ordering.
7d7465b  2026-06-10  Fixed unused Movelist list declaration
247af6d  2026-06-10  Fix pruning safety and search instability bugs
8ad12c3  2026-06-10  Add bishop pair, tempo bonus, and improved passed pawn eval
e90a54c  2026-06-10  Add king pawn shelter, backward pawn penalty, and rook on 7th bonus
dff0bbf  2026-06-10  Add knight outpost, rook behind passed pawn, and connected passers
f743ed3  2026-06-10  Fix bestmove 0000 with TT probe and fallback move generation
ba47efd  2026-06-10  Tune evaluation values to reduce endgame overvaluation
6db041c  2026-06-11  Merge branch 'EvaluationFixes'
7ece91e  2026-06-11  Merge branch 'SearchFixes'
cc69f0b  2026-06-12  Merge branch 'main' of https://github.com/AkashGupta-26/Polarity
077ad26  2026-06-12  Fix duplicate evaluation component counting
855ba6f  2026-06-12  Restore MopUp edge push, revert connected passers to per-pawn
f71533f  2026-06-12  Add king attack zone evaluation
c1e437f  2026-06-12  Add bad bishop penalty and blocked passer detection
7e3907a  2026-06-12  Add Texel tuner and apply blended tuned eval values
14ebc0f  2026-06-13  Retune eval on E12.52 dataset, upgrade tuner (+56 Elo)
0db9360  2026-06-13  Improve aspiration window scaling (gradual widening)
8cb05d4  2026-06-13  Add countermove history for better quiet move ordering
6f435c0  2026-06-13  Improve reverse futility pruning (extend to depth 6, tighter margins)
3ac81b3  2026-06-13  Never prune the TT/hash move in LMP/futility section
8c0f33b  2026-06-13  Improve endgame evaluation: free passer bonus, stronger mopup, drawish endgame scaling
9dfdf93  2026-06-13  Halve free passer bonus for rook pawns (a/h file)
58fa605  2026-06-14  Add structural eval improvements (+42 Elo)
af4c1ce  2026-06-14  Merge MatchMaker: add parallel UCI match runner
1e3bc23  2026-06-14  Update README with V11-V14 changes, expanded eval/search docs
```

*Note: These are the original commit hashes before the history rewrite. They no longer match the current repository.*