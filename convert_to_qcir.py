import sys

def qdimacs_to_qcir(qdimacs_file, qcir_file):
    # Read the QDIMACS file
    with open(qdimacs_file, 'r') as file:
        lines = file.readlines()

    qcir_content = ['#QCIR-G14']
    quantifiers = []
    gates = []
    clause_count = 0

    for line in lines:
        if line.startswith('c'):
            continue  # Skip comments
        elif line.startswith('p'):
            # Skip the preamble (handled implicitly)
            pass
        elif line[0] in 'ae':
            # Handle quantifiers
            parts = line.strip().split()
            quantifier = 'forall' if parts[0] == 'a' else 'exists'
            vars_list = ', '.join(parts[1:-1])
            quantifiers.append(f'{quantifier}({vars_list})')
        else:
            # Handle clauses
            literals = line.strip().split()[:-1]
            if len(literals) == 1:
                gate_expr = literals[0]
            else:
                gate_expr = f'or({", ".join(literals)})'
            gates.append(f'G{clause_count} = {gate_expr}')
            clause_count += 1

    # Define the final output gate (assuming all clauses must be satisfied)
    output_gate = f'and({", ".join("G" + str(i) for i in range(clause_count))})'
    gates.append(f'output = {output_gate}')

    qcir_content.extend(quantifiers)
    qcir_content.extend(gates)

    # Write to the QCIR file
    with open(qcir_file, 'w') as file:
        file.write('\n'.join(qcir_content))

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 transform_to_qcir.py input.qdimacs output.qcir")
    else:
        input_file = sys.argv[1]
        output_file = sys.argv[2]
        qdimacs_to_qcir(input_file, output_file)
