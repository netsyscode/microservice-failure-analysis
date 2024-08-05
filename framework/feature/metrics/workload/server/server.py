from flask import Flask, jsonify, request
import sys
import os

app = Flask(__name__)

@app.route('/api', methods=['GET'])
def api():
    return jsonify({"message": "Hello, World!"})

if __name__ == '__main__':
    print(os.getpid())
    app.run(host='0.0.0.0', port=int(sys.argv[1]), threaded=False)