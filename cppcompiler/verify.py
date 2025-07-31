#!/usr/bin/env python3
import random, re, subprocess

ops = ['+', '-', '*', '/', '%']

def gen_expr(depth=0):
    if depth > 2 or random.random() < 0.3:
        n = str(random.randint(0, 9))
        # maybe unary prefix
        if random.random() < 0.3:
            n = random.choice(['+', '-']) + n
        return n
    left = gen_expr(depth + 1)
    op = random.choice(ops)
    right = gen_expr(depth + 1)
    expr = f"{left} {op} {right}"
    if random.random() < 0.5:
        expr = f"( {expr} )"
    return expr

def to_rpn(tokens):
    prec = {'u+': 4, 'u-': 4, '*': 3, '/': 3, '%': 3, '+': 2, '-': 2}
    out, stack = [], []
    prev = None
    for t in tokens:
        # combined unary number, e.g. "+8" or "-3"
        m = re.fullmatch(r'([+-])(\d+)', t)
        if m:
            sign, num = m.groups()
            out.append(num)
            stack.append('u' + sign)
            prev = t
            continue

        # plain number
        if t.isdigit():
            out.append(t)

        # possible unary + or -
        elif t in ('+', '-') and (prev is None or prev in ops or prev == '('):
            stack.append('u' + t)

        # binary operator
        elif t in ops:
            while stack and stack[-1] != '(' and prec[stack[-1]] >= prec[t]:
                out.append(stack.pop())
            stack.append(t)

        # parentheses
        elif t == '(':
            stack.append(t)
        elif t == ')':
            while stack and stack[-1] != '(':
                out.append(stack.pop())
            if stack and stack[-1] == '(':
                stack.pop()

        prev = t

    # drain the stack
    while stack:
        top = stack.pop()
        if top != '(':
            out.append(top)
    return out

# continuously test until a mismatch
try:
	while True:
			expr = gen_expr()
			# write the test input
			with open('code.ferro','w') as f:
					f.write(expr + ' ;')

			# run ferro and split its RPN
			proc = subprocess.run(['./ferro'], capture_output=True, text=True)
			got = proc.stdout.strip().split()

			# generate expected RPN
			want = to_rpn(expr.split())

			if got != want:
					print("ERR")
					print("expr:  ", expr)
					print("expect:", ' '.join(want))
					print("got:   ", ' '.join(got))
					break
			else:
					print("OK:", expr, "→", ' '.join(got))
except KeyboardInterrupt:
	...
