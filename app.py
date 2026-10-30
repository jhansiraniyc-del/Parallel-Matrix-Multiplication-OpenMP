
from flask import Flask, render_template, request, jsonify
import subprocess
import json
import math
import os

app = Flask(__name__)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
PROGRAM = os.path.join(BASE_DIR, "matrix.exe")
MAX_N = 300


@app.route("/")
def home():
    return render_template("index.html")


@app.route("/multiply", methods=["POST"])
def multiply():
    try:
        data = request.get_json(force=True)

        n = int(data["n"])
        threads = int(data["threads"])
        A = data["A"]
        B = data["B"]

        if not 1 <= n <= MAX_N:
            return jsonify(error="Matrix size must be 1 to 300."), 400

        if not 1 <= threads <= 64:
            return jsonify(error="Invalid thread count."), 400

        for matrix in (A, B):
            if len(matrix) != n or any(len(row) != n for row in matrix):
                return jsonify(error="Matrix dimensions are invalid."), 400

            for row in matrix:
                for value in row:
                    if (isinstance(value, bool)
                        or not isinstance(value, (int, float))
                        or not math.isfinite(value)
                        or abs(value) >= 1e100):
                        return jsonify(error="Invalid matrix value."), 400

        parts = [f"{n} {threads}"]

        for matrix in (A, B):
            for row in matrix:
                parts.append(" ".join(str(x) for x in row))

        input_text = "\n".join(parts) + "\n"

        completed = subprocess.run(
            [PROGRAM],
            input=input_text,
            capture_output=True,
            text=True,
            timeout=60,
            check=True
        )

        result = json.loads(completed.stdout)

        seq = result["sequential_time"]
        par = result["parallel_time"]

        result["speedup"] = seq / par if par > 0 else None
        result["efficiency"] = (
            seq / (par * threads) * 100 if par > 0 else None
        )

        return jsonify(result)

    except subprocess.TimeoutExpired:
        return jsonify(error="Calculation timed out."), 408
    except subprocess.CalledProcessError:
        return jsonify(error="The C program failed."), 500
    except (ValueError, TypeError, KeyError, json.JSONDecodeError):
        return jsonify(error="Invalid input data."), 400
    except OSError:
        return jsonify(error="matrix.exe not found. Compile matrix.c first."), 500


if __name__ == "__main__":
    app.run(debug=True)
