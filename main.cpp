#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>
#include <cstdio>

const std::string ASCII_CHARS = "@#W$8%B&MMWXQO0ZKAHDGNbpqwmdoahk*+=|!;:,^`'. "; // Use these characters to print

int TARGET_FPS = 32;                          // Desired fps
int FRAME_DELAY_MS = 1000 / TARGET_FPS; 
int ASCII_WIDTH = 70;                  // Width of the ascii output
bool play = false;
void playAudioThread(std::string filename) {
    std::string cmd = "ffplay -nodisp -autoexit \"" + filename + "\" -loglevel quiet";
    system(cmd.c_str());
    play=true;
    // Just wait for ffplay to finish playing
    


}
void clearTerminal(){
    std::cout<<"\033[2J\033[1;1H";            // To clear the screen
}

char getAsciiChar(int r,int g,int b){
    int lum = static_cast<int>(0.299*r + 0.587*g + 0.114*b);    // Reads the input RGB colors and outputs an ASCII character associated with the brightness of the color
    return ASCII_CHARS[ASCII_CHARS.length() - 1 - (lum*ASCII_CHARS.length() / 256)];
}

std::string frameToAscii(const cv::Mat& frame){
    int origWidth = frame.cols;         
    int origHeight = frame.rows;
    float aspect = static_cast<float>(origWidth) / origHeight;
    int newHeight = static_cast<int>(ASCII_WIDTH / aspect / 2.0);

    cv::Mat resized;
    cv::resize(frame, resized,cv::Size(ASCII_WIDTH,newHeight));
    
    std::string output;
    std::ostringstream oss;
    for(int y=0;y<resized.rows;++y){
        for(int x=0;x<resized.cols;++x){
            cv::Vec3b pixel = resized.at<cv::Vec3b>(y,x);         // Iterate through each pixel of the frame
            int b = pixel[0]; int g = pixel[1]; int r = pixel[2]; // Extract color information out of pixel
            char asciiChar = getAsciiChar(r,g,b);                 // Converts color to ascii character
            
            oss<<"\033[38;2;"<<(r)<<";"<<           // Prints the ascii character along with its color information on terminal
                               (g)<<";"<<        
                               (b)<<"m"<<asciiChar;
        }
        oss << "\n";
    }
    oss<<"\033[0m";
    output+=oss.str();
    return output;
}
int main(int argc, char** argv){
    std::string videofile = argv[1];                            // Video path
    std::thread audioThread(playAudioThread, videofile);
    ASCII_WIDTH = std::stoi(argv[2]);
    
    // Detach so it plays independently (main thread can continue)
    audioThread.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    cv::VideoCapture cap(videofile);                            // Video file
    if(!cap.isOpened()){
        std::cerr << "Error: Cannot open the video file\n";
        return 1;
    }

    double videoFPS = cap.get(cv::CAP_PROP_FPS);                // idk
    TARGET_FPS = std::round(videoFPS * 1.32f);
    FRAME_DELAY_MS = 1000 / TARGET_FPS;
    int frameskip = std::max(static_cast<int>(videoFPS/TARGET_FPS),1); //number of frames to skip after each frame
    int frameCount = 0;
    
    
    cv::Mat frame;
    while(true){
        auto start = std::chrono::high_resolution_clock::now();
        if(!cap.read(frame)) break;

        if(frameCount++ % frameskip != 0) continue;

        
        std::string ascii = frameToAscii(frame);
        clearTerminal();
        std::cout << TARGET_FPS << std::endl;
        std::cout << ascii << std::flush;           // Here is where you'll see the text being printed to terminal

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
        
        if(elapsed < FRAME_DELAY_MS){
        std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_DELAY_MS - elapsed));
        } else {
            frameCount++;
            
        }
        
    }
    cap.release();  // Closes the video file
    return 0;
}