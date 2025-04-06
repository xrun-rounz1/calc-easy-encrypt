import json
import sys

jsonstr = sys.stdin.read(-1)

obj = json.loads(jsonstr)

values = []

for item in obj["data"]["mylist"]["items"]:
    values.append("  ('{}', '{}')".format(
        item["watchId"][2:],
        item["video"]["registeredAt"].replace("+09:00", "")
    ))

print("INSERT INTO video VALUES")
print(",\n".join(values))
print(";")