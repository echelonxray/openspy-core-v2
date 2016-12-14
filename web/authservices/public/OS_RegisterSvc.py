from cgi import parse_qs, escape
import xml.etree.ElementTree as ET

import binascii
from M2Crypto import RSA, BIO
import md5, struct, os

from collections import OrderedDict
import jwt

from BaseService import BaseService
import httplib, urllib, json

import redis

class OS_RegisterSvc(BaseService):
    # expects following vars:
    #   partnercode - 0
    #   email - if only email provided its a userid login
    #   email uniquenick namespaceid - profile login
    #   email nick namespaceid - profile login
    def try_register(self, register_options):

    	#perform user registration
        passthrough_user_params = ["password", "email", "partnercode"]
        user_data = {}
        for param in passthrough_user_params:
        	if param in register_options:
        		user_data[param] = register_options[param]

        params = jwt.encode(user_data, self.SECRET_REGISTER_KEY, algorithm='HS256')
        #params = urllib.urlencode(params)
        
        headers = {"Content-type": "application/x-www-form-urlencoded","Accept": "text/plain"}
        conn = httplib.HTTPConnection(self.REGISTER_SERVER)

        conn.request("POST", self.REGISTER_SCRIPT, params, headers)
        response = conn.getresponse().read()
        response = jwt.decode(response, self.SECRET_REGISTER_KEY, algorithm='HS256')

        #perform profile creation
        if response["success"]:
        	user = response['user']
	        passthrough_profile_params = ["nick", "uniquenick", "namespaceid"]

	        profile = {}
	        for key in passthrough_profile_params:
	            if key in register_options:
	                profile[key] = register_options[key]

	        request_data = {'userid': user['id'], 'mode': 'create_profile', 'profile': profile}

	        params = jwt.encode(request_data, self.SECRET_PROFILEMGR_KEY, algorithm='HS256')	        
	        
	        headers = {"Content-type": "application/x-www-form-urlencoded","Accept": "text/plain"}

	        conn = httplib.HTTPConnection(self.PROFILE_MGR_SERVER)

	        conn.request("POST", self.PROFILE_MGR_SCRIPT, params, headers)
	        response = conn.getresponse().read()
	        response = jwt.decode(response, self.SECRET_PROFILEMGR_KEY, algorithm='HS256')
	        response["user"] = user

	        #create auth session
	        login_data = {}

	        login_data["save_session"] = True
	        login_data["email"] = register_options["email"]
	        login_data["password"] = register_options["password"]
	        login_data["partnercode"] = register_options["partnercode"]


	        params = jwt.encode(login_data, self.SECRET_AUTH_KEY, algorithm='HS256')
	        #params = urllib.urlencode(params)
	        
	        headers = {"Content-type": "application/x-www-form-urlencoded","Accept": "text/plain"}
	        conn = httplib.HTTPConnection(self.LOGIN_SERVER)

	        conn.request("POST", self.LOGIN_SCRIPT, params, headers)
	        response = conn.getresponse().read()
	        response = jwt.decode(response, self.SECRET_AUTH_KEY, algorithm='HS256')

        return response
    def check_user_conflicts(self, request):
        headers = {"Content-type": "application/x-www-form-urlencoded","Accept": "text/plain"}

        conn = httplib.HTTPConnection(self.USER_MGR_SERVER)

        params = {}
        params['email'] = request['email']
        params['partnercode'] = request['partnercode']
        params['mode'] = 'get_user'

        params = jwt.encode(params, self.SECRET_USERMGR_KEY, algorithm='HS256')

        conn.request("POST", self.USER_MGR_SCRIPT, params, headers)
        response = conn.getresponse().read()
        response = jwt.decode(response, self.SECRET_USERMGR_KEY, algorithm='HS256')
        return "user" in response
    def check_profile_conflicts(self, request):
        headers = {"Content-type": "application/x-www-form-urlencoded","Accept": "text/plain"}

        conn = httplib.HTTPConnection(self.PROFILE_MGR_SERVER)

        params = {}
        params['uniquenick'] = request['uniquenick']
        if request['namespaceid'] != 0:
            params['namespaceid'] = request['namespaceid']
        params['mode'] = 'get_profile'

        params = jwt.encode(params, self.SECRET_PROFILEMGR_KEY, algorithm='HS256')

        conn.request("POST", self.PROFILE_MGR_SCRIPT, params, headers)
        response = conn.getresponse().read()
        response = jwt.decode(response, self.SECRET_PROFILEMGR_KEY, algorithm='HS256')
        return "profile" in response and response["profile"] != None
    def run(self, env, start_response):
        # the environment variable CONTENT_LENGTH may be empty or missing
        try:
            request_body_size = int(env.get('CONTENT_LENGTH', 0))
        except (ValueError):
            request_body_size = 0

        # When the method is POST the variable will be sent
        # in the HTTP request body which is passed by the WSGI server
        # in the file like wsgi.input environment variable.
        request_body = json.loads(env['wsgi.input'].read(request_body_size))
       # d = parse_qs(request_body)

        start_response('200 OK', [('Content-Type','text/html')])

        print("Register: {}\n".format(request_body))
        #Register: {u'uniquenick': u'sctest01', u'namespaceid': u'0', u'nick': u'sctest01', u'mode': u'create_account', u'partnercode': u'0', u'password': u'gspy', u'email': u'sctest@gamespy.com'}

        response = {}
        required_params = ["email", "partnercode", "password", "uniquenick", "nick", "namespaceid"]
        for key in required_params:
            if key not in request_body:
                response['success'] = False
                response['error'] = "MISSING_PARAMS"
                return json.dumps(response)

        if self.check_user_conflicts(request_body):
            response['success'] = False
            response['error'] = "USER_EXISTS"
        elif self.check_profile_conflicts(request_body):
            response['success'] = False
            response['error'] = "UNIQUENICK_EXISTS"
            

        if 'success' in response:
            return json.dumps(response)



        response = self.try_register(request_body)


        return json.dumps(response)