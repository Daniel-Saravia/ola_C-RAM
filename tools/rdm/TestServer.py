#!/usr/bin/python
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Library General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# TestServer.py
# Copyright (C) 2010 Simon Newton

from wsgiref.simple_server import make_server
import json

settings = {
  'PORT': 9999,
  'headers': [('Content-type', 'application/json')],
}

status = {
  '200': '200 OK',
  '404': '404 Not Found',
}

paths = {
  '/RunTests': 'run_tests'
}

"""
  An instance of this class is created to serve every request.
"""
class TestServerApplication:
  def __init__(self, environ, start_response):
    self.environ = environ
    self.start = start_response
    self.get_params = {}
    self.__request_handler()
  
  def __request_handler(self):
    self.request = self.environ['PATH_INFO']

    if self.request not in paths.keys():
      self.status = status['404']
    else:
      self.status = status['200']
      params = self.environ['QUERY_STRING'].split('&')

      for param in params:
        param = param.split('=')
        if len(param) > 1:
          self.get_params[str(param[0])] = str(param[1])

    return self.__response_handler()

  def __response_handler(self):
    if self.status == status['404']:
      self.response = json.dumps({'status': False, 'message': 'Invalid request!'})
    elif self.status == status['200']:
      self.response = json.dumps(self.get_params)

  def __iter__(self):
    self.start(self.status, settings['headers'])
    yield(self.response)


 
class TestServer:
  
  def __init__(self, options):
    """
      Prepare test environment and initialize tests
    """
    None

  def run_tests(self):
    None



def parse_options():
  """
    Parse Command Line options
  """
  return {}

def main():
  options = parse_options()
  settings.update(options)
  httpd = make_server('', settings['PORT'], TestServerApplication)
  httpd.serve_forever()

if __name__ == '__main__':
  main()
