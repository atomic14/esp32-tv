import cv2

def resize_cover(frame, target_size):
    target_width, target_height = target_size
    frame_height, frame_width = frame.shape[:2]

    # Calculate the aspect ratio
    aspect_ratio = frame_width / frame_height

    # Calculate new dimensions
    new_width = int(target_height * aspect_ratio)
    new_height = target_height

    if new_width < target_width:
        new_width = target_width
        new_height = int(target_width / aspect_ratio)

    # Resize the frame
    frame_resized = cv2.resize(frame, (new_width, new_height))

    # Crop to the target size
    x_offset = (new_width - target_width) // 2
    y_offset = (new_height - target_height) // 2

    frame_cropped = frame_resized[y_offset:y_offset + target_height, x_offset:x_offset + target_width]

    return frame_cropped