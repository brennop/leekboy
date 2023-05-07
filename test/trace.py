import sys

red_color = "\033[1;31m"
green_color = "\033[1;32m"

expected, actual = sys.argv[1], sys.argv[2]

with open(expected) as f, open(actual) as g:
    # skip lines that do not contain the trace
    for line in f:
        if line.startswith("A"):
            break

    for line in g:
        if line.startswith("A"):
            break

    prev = ""
    for line, (expected_line, actual_line) in enumerate(zip(f, g)):
        exp_data = expected_line.lower().split()
        act_data = actual_line.lower().split()

        error_at = None
        for i in range(0, 7):
            if exp_data[i] != act_data[i]:
                error_at = i
                break

        if error_at is not None:
            print(f"Error at line {line}:")
            print(f"Expected: {expected_line}", end="")
            print(f"Actual:   {actual_line}")
            print(f"Previous: {prev}")
            break

        prev = actual_line
            
