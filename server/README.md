# Setup

Make sure you have python3 installed.

Then create a virtual environment:

```bash
python3 -m venv venv
```

Activate the virtual environment:

```bash
source venv/bin/activate
```

Install the requirements:

```bash
pip install -r requirements.txt
```

# Adding Vidoes

Place videos in the `movies` directory. The videos should be in the mp4 format and should be fairly short (around 2-3 minutes maximum).

If you change any parameters for the video (e.g. the frame size), just detele the `cache` directory and it will be regenerated when you start the server.

# Running

To run the application:

```bash
source venv/bin/activate
python3 app.py
```