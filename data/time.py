# analyze the output of json file tmp/state.json

import json

with open('tmp/state.json') as f:
    data = json.load(f)

# find all data['timeline']['event_time']
event_time = []
for i in data['timeline']:
    if 'event_time' in i:
        event_time.append(i['event_time'])
        print(i['event_time'])
        