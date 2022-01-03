f = open("./trojan.c", "r")
buff= f.read()

for c in buff:
	if c == '\\':
		print("\\\\", end='')
	elif c == '"':
		print("\\\"", end='')
	elif c == '\n':
		print("\\n\\\n", end='')
	else: 
		print(c, end='')