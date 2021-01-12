// DeltaTime.cpp written by greg-bt 2021
// Written with OpenCV

#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

// System variables
bool active  = true;
int fpsCap   = 200,     // Maximum framerate
    tickRate = 120,     // Desired tickrate
     
    scale    = 600,     // Size of output frame
    resolution = 6;     // Size of subdivisions ( for history )

// Calculate time since last physics tick
int deltaTime(int previous, int offset) {
    return (clock() - previous) + offset;
}

// Physics object class
class PhysicsPoint {
public:
    // Kinematic variables
    float v, s, a;
    // History pointer
    int pointer = 0;

    // Constructor
    PhysicsPoint(float velocityIn, float displacementIn, float accelerationIn, Scalar colorIn) {
        v = velocityIn; s = displacementIn; a = accelerationIn;
        history.assign(scale / resolution, 0);
        color = colorIn;
    }

    vector<int> getHistory() { return history; }

    // Physics function
    void update() {
        // Apply acceleration to object
        v += a;
        s += v;

        // Check for collisions with floor
        if (s < 20) {
            v = -v * 0.8;
            s = 20;
        }

        // Update displacement history
        history[pointer] = (int)s;
        pointer++;

        if (pointer >= history.size()) {
            pointer = 0; // Prevent index out of bounds
        }
    }

private:
    vector<int> history;    // Record of previous displacements
    Scalar color;           // Color of dot
};

// Carry out physics ticks
void physicsTick(vector<PhysicsPoint> & objects ) {

    // Update all physics objects
    for (int i = 0; i < objects.size(); i++) {
        objects[i].update();
    }

}

// Create image output
Mat render(vector<PhysicsPoint> objects ) {

    // Create blank frame
    Mat frame = Mat::zeros(scale, scale, CV_8UC3);

    // Draw each object
    for (int i = 0; i < objects.size(); i++) {

        PhysicsPoint point = objects[i];
        vector<int> history = point.getHistory();

        // Draw history
        for (int i = 0; i < history.size(); i++) {
            // Draw circle
            circle(frame,   // Target Image
                { i * (int)(scale / history.size() / 2), // x position
                    scale - history[point.pointer] },   // y position
                2, Scalar(0, 0, 255), 1);    // Radius, Color & Thickness
            point.pointer++;
            if (point.pointer >= history.size()) point.pointer = 0;
        }

        // Draw velocity indicator
        line(frame, // Target Image
            { scale / 2, scale - (int)point.s },
            { scale / 2, scale - (int)point.s - (int)point.v * 10 },
            Scalar(0, 255, 255), 2); // Color & Thickness

        // Draw position from last physics tick
        circle(frame, { scale / 2, scale - (int)point.s }, 2, Scalar(0, 255, 0), 3);

    }

    // Return rendered frame
    return frame;
}

// Add velocity to physics objects
void bump(vector<PhysicsPoint> &objects, bool direction) {
    for (int i = 0; i < objects.size(); i++) {
        objects[i].v = (direction ? i : -i)+3;
    }
}

int main() {
    // Create display window
    namedWindow("DeltaTime", WINDOW_AUTOSIZE);
    createTrackbar("FrameRate", "DeltaTime", &fpsCap, 240);
    createTrackbar("TickRate ", "DeltaTime", &tickRate, 200);

    // Initial time variables
    int ptime = clock();    // Time since last frame
    int offset = 0;         // Time since last physics tick

    vector<PhysicsPoint> points = { PhysicsPoint(8,20, -0.0981, Scalar(0, 255, 0)) };

    // Framerate controlled loop
    while (active) {

        // Calculate delta time accounting for any offset time
        int delta = deltaTime(ptime, offset);
        ptime = clock(); // Current time becomes time of last frame

        // Do physics ticks
        for (int i = 0; i < delta / (1000 / tickRate); i++) {
            physicsTick(points);
        }

        // Calculate the offset time
        offset = delta % (1000 / tickRate);

        // Display the frame
        imshow(
            "DeltaTime",
            render(points) // Render the frame
        );

        int x = waitKey(1000 / (fpsCap ? fpsCap : 1));
        if (x == 119) bump(points, true);        // w key ( +ve )
        else if (x == 115) bump(points, false);  // s key ( -ve )

        else if (x == 32) {     // space key ( add new object )
            RNG rng(12345);
            points.push_back(PhysicsPoint(8, 20, -0.0981, Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256))));
        }
        else if (x >= 0) break;
    }
}