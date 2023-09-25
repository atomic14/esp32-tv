#!/bin/bash

# Check if input and output files are provided
if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]; then
    echo "Usage: $0 <input_file> <output_file> <frame_size (e.g. 280:240)>"
    exit 1
fi

# Input and output filenames
input_file="$1"
output_file="$2"
frame_size="$3"

# First pass: Analyze audio and extract loudness statistics
echo "Performing first pass to analyze audio..."
loudness_data=$(ffmpeg -i "$input_file" -af "loudnorm=print_format=json" -vn -sn -dn -f null -y /dev/null 2>&1 | awk '/{/,/}/' | jq -c '.')

# Extract values from JSON
input_i=$(echo "$loudness_data" | jq -r '.input_i')
input_tp=$(echo "$loudness_data" | jq -r '.input_tp')
input_lra=$(echo "$loudness_data" | jq -r '.input_lra')
input_thresh=$(echo "$loudness_data" | jq -r '.input_thresh')

# Second pass: Normalize audio using extracted values
echo "Performing second pass to normalize audio..."
ffmpeg -i "$input_file" -vf "scale=$frame_size:force_original_aspect_ratio=increase,crop=$frame_size" -af "loudnorm=measured_i=$input_i:measured_tp=$input_tp:measured_lra=$input_lra:measured_thresh=$input_thresh:offset=0.0:linear=true:print_format=summary" -r 15 -c:v mjpeg -q:v 10 -acodec pcm_u8 -ar 16000 -ac 1 "$output_file"

echo "Normalization complete. Output saved as $output_file"
