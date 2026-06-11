"""Extract quiet positions from PGN files for Texel tuning.
Outputs: FEN [result] per line, one position every few moves after move 8."""

import sys
import re
import os

def parse_pgn(filepath):
    """Yield (moves_text, result) for each game in a PGN file."""
    with open(filepath, 'r') as f:
        content = f.read()

    games = re.split(r'\n\n(?=\[)', content)
    for game_block in games:
        result_match = re.search(r'\[Result\s+"([^"]+)"\]', game_block)
        if not result_match:
            continue
        result = result_match.group(1)
        if result not in ('1-0', '0-1', '1/2-1/2'):
            continue

        header_end = game_block.rfind(']')
        if header_end == -1:
            continue
        moves_text = game_block[header_end + 1:].strip()
        moves_text = re.sub(r'\{[^}]*\}', '', moves_text)
        moves_text = re.sub(r'\d+\.+', '', moves_text)
        moves_text = re.sub(r'(1-0|0-1|1/2-1/2|\*)', '', moves_text)
        moves_text = moves_text.strip()

        yield moves_text, result

def result_to_score(result):
    if result == '1-0':
        return '1.0'
    elif result == '0-1':
        return '0.0'
    else:
        return '0.5'

def extract_fens_from_pgn_simple(pgn_path, output_positions, sample_every=4, min_move=8):
    """Extract FEN positions by replaying games with the engine.
    Since we don't have python-chess, we'll use a simpler approach:
    extract positions from PGN comments that contain FEN, or
    generate positions by running the engine."""
    count = 0
    for moves, result in parse_pgn(pgn_path):
        tokens = moves.split()
        if len(tokens) < min_move * 2:
            continue
        score = result_to_score(result)
        # We can't replay without python-chess, so note the game for manual extraction
        count += 1

    return count

def main():
    if len(sys.argv) < 3:
        print("Usage: python extract_positions.py <pgn_dir_or_file> <output.txt>")
        print("\nAlternative: generate positions via engine self-play:")
        print("  python extract_positions.py --selfplay <engine_path> <output.txt> [num_games]")
        sys.exit(1)

    if sys.argv[1] == '--selfplay':
        if len(sys.argv) < 4:
            print("Usage: python extract_positions.py --selfplay <engine_path> <output.txt> [num_games]")
            sys.exit(1)
        engine_path = sys.argv[2]
        output_path = sys.argv[3]
        num_games = int(sys.argv[4]) if len(sys.argv) > 4 else 100
        generate_selfplay_positions(engine_path, output_path, num_games)
    else:
        pgn_input = sys.argv[1]
        output_path = sys.argv[2]
        print(f"Note: For proper FEN extraction, install python-chess: pip install chess")
        try:
            import chess
            import chess.pgn
            extract_with_python_chess(pgn_input, output_path)
        except ImportError:
            print("python-chess not installed. Install with: pip install chess")
            print("Then re-run this script.")
            sys.exit(1)

def extract_with_python_chess(pgn_input, output_path, sample_every=4, min_move=8):
    import chess
    import chess.pgn

    pgn_files = []
    if os.path.isdir(pgn_input):
        for f in os.listdir(pgn_input):
            if f.endswith('.pgn'):
                pgn_files.append(os.path.join(pgn_input, f))
    else:
        pgn_files.append(pgn_input)

    positions = []
    total_games = 0

    for pgn_path in pgn_files:
        print(f"Processing {pgn_path}...")
        with open(pgn_path) as f:
            while True:
                game = chess.pgn.read_game(f)
                if game is None:
                    break

                result = game.headers.get('Result', '*')
                if result not in ('1-0', '0-1', '1/2-1/2'):
                    continue

                score = result_to_score(result)
                board = game.board()
                move_num = 0

                for move in game.mainline_moves():
                    board.push(move)
                    move_num += 1

                    if move_num < min_move * 2:
                        continue
                    if move_num % sample_every != 0:
                        continue

                    # Skip non-quiet positions
                    if board.is_check():
                        continue

                    fen = board.fen()
                    positions.append(f"{fen} [{score}]")

                total_games += 1

    with open(output_path, 'w') as f:
        for pos in positions:
            f.write(pos + '\n')

    print(f"Extracted {len(positions)} positions from {total_games} games")
    print(f"Written to {output_path}")

def generate_selfplay_positions(engine_path, output_path, num_games):
    """Generate positions by having the engine play itself via UCI."""
    import subprocess
    import chess
    import chess.engine

    positions = []
    print(f"Generating {num_games} self-play games...")

    with chess.engine.SimpleEngine.popen_uci(engine_path) as engine:
        for i in range(num_games):
            board = chess.Board()
            move_num = 0

            while not board.is_game_over() and move_num < 200:
                result = engine.play(board, chess.engine.Limit(time=0.1))
                board.push(result.move)
                move_num += 1

            game_result = board.result()
            if game_result not in ('1-0', '0-1', '1/2-1/2'):
                continue

            score = result_to_score(game_result)

            # Replay and extract quiet positions
            replay_board = chess.Board()
            for j, move in enumerate(board.move_stack):
                replay_board.push(move)
                if j < 16 or j % 4 != 0:
                    continue
                if replay_board.is_check():
                    continue
                positions.append(f"{replay_board.fen()} [{score}]")

            if (i + 1) % 10 == 0:
                print(f"  {i + 1}/{num_games} games complete, {len(positions)} positions")

    with open(output_path, 'w') as f:
        for pos in positions:
            f.write(pos + '\n')

    print(f"Generated {len(positions)} positions from {num_games} games")
    print(f"Written to {output_path}")

if __name__ == '__main__':
    main()
