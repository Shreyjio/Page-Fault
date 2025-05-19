from flask import Flask, render_template, send_file, request
import subprocess

app = Flask(__name__)

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/run-monitor', methods=['GET', 'POST'])
def run_monitor():
    try:
        subprocess.run(["./a.exe"], timeout=20, check=True)
        with open("page_fault_log.txt", "r") as file:
            content = file.read()
        return f"<pre>{content}</pre>"
    except subprocess.TimeoutExpired:
        return "Monitoring took too long and was terminated."
    except Exception as e:
        return f"Error running monitor: {e}"

@app.route('/generate-graph', methods=['GET'])
def generate_graph():
    try:
        subprocess.run(["python", "plot_graph.py"], check=True)
        return send_file("static/graph.png", mimetype='image/png')
    except Exception as e:
        return f"Error generating graph: {e}"

if __name__ == '__main__':
    app.run(debug=True)