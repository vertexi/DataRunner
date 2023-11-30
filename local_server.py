from http.server import SimpleHTTPRequestHandler
import socketserver
import sys


DIRECTORY = "build/"

class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)
    extensions_map = {
        '': 'application/octet-stream',
        '.css':	'text/css',
        '.html': 'text/html',
        '.jpg': 'image/jpg',
        '.js':	'application/x-javascript',
        '.json': 'application/json',
        '.manifest': 'text/cache-manifest',
        '.png': 'image/png',
        '.wasm':	'application/wasm',
        '.xml': 'application/xml',
    }

    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        SimpleHTTPRequestHandler.end_headers(self)

if __name__ == '__main__':
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
    with socketserver.TCPServer(("localhost", port), Handler) as httpd:
        print("Serving on port", port)
        httpd.serve_forever()
