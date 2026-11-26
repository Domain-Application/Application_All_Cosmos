import re, sys
s = open(sys.argv[1], encoding='utf-8').read()
s = s.replace(r'\u0022', '"').replace(r'\u003E', '>').replace(r'\u003C', '<').replace(r'\u0026', '&').replace(r'\u0027', "'").replace(r'\r\n', '\n')
open(sys.argv[2], 'w', encoding='utf-8').write(s)
for m in re.finditer(r'"property": "(\w+)",\s*"groupId": [^,]*,\s*"title": "([^"]*)"', s):
    print(m.group(1), '|', m.group(2))
