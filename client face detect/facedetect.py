import cv2
from pymongo import MongoClient
from datetime import datetime
import threading
import time

# MongoDB connection (Replace with your MongoDB cluster connection string)
client = MongoClient('mongodb+srv://hci_user:oUoaJXKsn2MFCeHU@cluster0.gbt6n.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0')
db = client['face_detection_db']
collection = db['detection_status']

# Load Haar Cascade for face detection
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

# Open webcam
cap = cv2.VideoCapture(0)

# Insert detection status asynchronously
def insert_detection_status(status):
    collection.insert_one({
        'status': status,
        'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    })

# Variable to track when the last check was performed
last_check_time = time.time()

try:
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame")
            break

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        faces = face_cascade.detectMultiScale(gray, scaleFactor=1.3, minNeighbors=5)

        # Draw rectangles around detected faces
        for (x, y, w, h) in faces:
            cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)

        cv2.imshow('Face Detection', frame)

        # Check if 20 seconds have passed since the last face detection check
        if time.time() - last_check_time >= 10:
            last_check_time = time.time()  # Update last check time
            
            # Determine detection status
            status = 'No Face Detected' if len(faces) == 0 else 'Face Detected'
            
            # Run MongoDB insertion in a separate thread
            threading.Thread(target=insert_detection_status, args=(status,)).start()
            print(f"Status: {status} at {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

        # Exit condition
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    print("Stopped by user")

finally:
    cap.release()
    cv2.destroyAllWindows()
