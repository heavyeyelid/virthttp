import requests
import xml.etree.ElementTree as ET
import os
from dotenv import load_dotenv

load_dotenv()

API_URI = os.getenv("API_URI") + "/domains"
TEST_DOMAIN_NAME = os.getenv("TEST_DOMAIN_NAME")
ARCH = os.getenv('ARCH')

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

domain_source_xml = ET.parse('domain-' + TEST_DOMAIN_NAME + '-dumpxml.xml').getroot()
domain_dumped_xml = ET.fromstring(json_res["results"][0][0])
if ET.tostring(domain_source_xml) != ET.tostring(domain_dumped_xml):
    exit(6)

json_res = requests.get(API_URI + "/by-uuid/" + domain_uuid + "/max_memory").json()
if not json_res["success"] or json_res["results"][0][0] != 131072:
    exit(7)


def action_table_test(key, action):
    res = requests.patch(API_URI + '/by-uuid/' + domain_uuid, json={key: action}).json()
    if not res["success"]:
        exit(8)
    res = requests.get(API_URI + '/by-uuid/' + domain_uuid).json()
    if not res["success"]:
        exit(9)
    return res["results"][0]


def pw_mgt_test(action):
    pw_status = 'Running' if action == 'start' or action == 'resume' or action == 'reset' or action == 'reboot' \
        else 'Shutoff' if action == 'destroy' or action == 'shutdown' else 'Paused'
    if action_table_test('power_mgt', action)["status"] != pw_status:
        exit(10)


if action_table_test('max_memory', 196608)["ram_max"] != 196608:
    exit(11)

pw_mgt_test('start')

if action_table_test('memory', 196608)["ram"] != 196608:
    exit(12)

json_res = requests.get(API_URI + '/by-uuid/' + domain_uuid + '/max_vcpus').json()
if not json_res["success"] or json_res["results"][0][0] != 1:
    exit(13)

json_res = requests.get(API_URI + '/by-uuid/' + domain_uuid + '/num_vcpus').json()
if not json_res["success"] or json_res["results"][0][0] != 1:
    exit(14)

pw_mgt_test('reset')
pw_mgt_test('suspend')
pw_mgt_test('resume')
pw_mgt_test('destroy')

if action_table_test('name', TEST_DOMAIN_NAME + '_renamed')["name"] != TEST_DOMAIN_NAME + '_renamed':
    exit(15)
if not requests.patch(API_URI + '/by-uuid/' + domain_uuid, json={"name": TEST_DOMAIN_NAME}).json()["success"]:
    exit(16)

if not action_table_test('autostart', True)["autostart"]:
    exit(17)

# if action_table_test('send_signal', )

# if action_table_test('send_keys')

if not requests.delete(API_URI + '/by-uuid/' + domain_uuid).json()["success"]:
    exit(18)

json_res = requests.get(API_URI).json()
if next((o for o in json_res["results"] if o["name"] == TEST_DOMAIN_NAME), None) is not None:
    exit(19)

domain_xml = open('domain-' + ARCH + '-host.xml', 'r')
if not requests.post(API_URI, json=domain_xml.read()).json()["success"]:
    exit(20)

json_res = requests.get(API_URI).json()
test_domain = next((o for o in json_res["results"] if o["name"] == TEST_DOMAIN_NAME), None)
if test_domain is None:
    exit(21)
domain_uuid = test_domain["uuid"]

if not requests.patch(API_URI + '/by-uuid/' + domain_uuid, json={"power_mgt": "destroy"}).json()["success"]:
    exit(22)