#!/usr/bin/env python3

import urllib.request, json, sys

url = 'https://icecast_server/status-json.xsl'


# Get listeners on mount point
def get_listeners(mount_point):

    try:

        # json parsing
        request = urllib.request.urlopen(url)
        content = request.read().decode('utf8')
        data = json.loads(content)

        mount = data['icestats']['source']

        for c in range(len(mount)):
            mount_url = mount[c]['listenurl']
            # print(mount_url) # debug

            if mount_url == ('http://localhost:8000/{}'.format(mount_point)):
                listeners = mount[c]['listeners']
                # print('listeners: ', listeners) # debug
                print(listeners)

    except:
        print("Something went wrong")


# Get total listeners
def total_listeners():

    total = 0

    try:

        # json parsing
        request = urllib.request.urlopen(url)
        content = request.read().decode('utf8')
        data = json.loads(content)

        mount = data['icestats']['source']

        for c in range(len(mount)):
            listeners = mount[c]['listeners']
            total = total + listeners
            # print(mount[c]['listenurl']) # debug

        # print('total ', total) @ debug
        print(total)

    except:
        print("Something went wrong")


# Main function
if len(sys.argv) > 1:
    get_listeners(sys.argv[1])

else:

    total_listeners()
