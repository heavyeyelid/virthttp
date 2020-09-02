import requests
import xml.etree.ElementTree as ET
import os
from dotenv import load_dotenv

load_dotenv()

API_URI = os.getenv("API_URI") + "/domains"
TEST_DOMAIN_NAME = os.getenv("TEST_DOMAIN_NAME")

json_res = requests.get(API_URI).json()
if not json_res["success"]:
    exit(1)

test_domain = next((o for o in json_res["results"] if o["name"] == TEST_DOMAIN_NAME), None)
if not test_domain or test_domain["id"] != -1 or test_domain["status"] != "Shutoff" or test_domain["os"] != "hvm" or \
        test_domain["ram"] != 131072 or test_domain["ram_max"] != 131072 or test_domain["cpu"] != 1:
    exit(2)

domain_uuid = test_domain["uuid"]

json_res = requests.get(API_URI + "/by-name/" + TEST_DOMAIN_NAME).json()
if not json_res["success"] or json_res != requests.get(API_URI + "/by-uuid/" + domain_uuid).json():
    exit(3)

test_domain = json_res["results"][0]
if not test_domain or test_domain["name"] != TEST_DOMAIN_NAME or test_domain["uuid"] != domain_uuid:
    exit(4)

json_res = requests.get(API_URI + "/by-name/" + TEST_DOMAIN_NAME + "/xml_desc").json()
if not json_res["success"]:
    exit(5)

xml_dump = ET.parse('domain-' + TEST_DOMAIN_NAME + '-dumpxml.xml').getroot()

xml_desc = ET.fromstring(json_res["results"][0][0])
if ET.tostring(xml_dump) != ET.tostring(xml_desc):
    exit(6)


def action_table_test(key, action):
    res = requests.patch(API_URI + '/by-uuid/' + domain_uuid, json={key: action}).json()
    if not res["success"]:
        exit(7)
    res = requests.get(API_URI + '/by-uuid/' + domain_uuid).json()
    if not res["success"]:
        exit(8)
    return res["results"][0]


def pw_mgt_test(action):
    pw_status = 'Running' if action == 'start' or action == 'resume' or action == 'reset' or action == 'reboot' \
        else 'Shutoff' if action == 'destroy' or action == 'shutdown' else 'Paused'
    if action_table_test('power_mgt', action)["status"] != pw_status:
        exit(9)


pw_mgt_test('start')
pw_mgt_test('reset')
pw_mgt_test('suspend')
pw_mgt_test('resume')
pw_mgt_test('destroy')

if action_table_test('name', TEST_DOMAIN_NAME + '_renamed')["name"] != TEST_DOMAIN_NAME + '_renamed':
    exit(10)
if not requests.patch(API_URI + '/by-uuid/' + domain_uuid, json={"name": TEST_DOMAIN_NAME}).json()["success"]:
    exit(10)

if action_table_test('memory', 196608)["ram"] != 196608:
    exit(11)

if action_table_test('max_memory', 262144)["max_memory"]["ram_max"] != 262144:
    exit(12)

if not action_table_test('autostart', True)["autostart"]:
    exit(13)

# if action_table_test('send_signal', )

# if action_table_test('send_keys')
