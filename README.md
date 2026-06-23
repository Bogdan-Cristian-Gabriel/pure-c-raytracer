# Multithreaded CPU Raytracer in C

A from-scratch, CPU-based 3D raytracing engine written entirely in C. It renders spheres with diffuse lighting, hard shadows, and recursive reflections, while allowing for real-time interactive camera movement in a 3D space.

<img width="810" height="630" alt="image" src="https://github.com/user-attachments/assets/6975f269-1ecd-437d-a932-695d7eb367f9" />


## 🚀 Key Features

* **Custom Math Engine:** Built a custom 3D vector math library from scratch (linear algebra, dot products, cross products, normalization, and 3D rotation matrices).
* **Multithreading (POSIX Threads):** The rendering pipeline is parallelized across multiple CPU cores by dividing the screen into horizontal tiles, dramatically increasing real-time performance.
* **Global Illumination & Optics:**
  * Lambertian diffuse shading.
  * Hard shadows generated via secondary shadow rays.
  * Recursive reflections for mirror-like surfaces.
  * Epsilon offsetting to prevent shadow acne.
* **Real-Time Interactivity:** Integrated SDL2 to handle window creation, framebuffers, and raw mouse/keyboard input for a First-Person camera experience (Pitch, Yaw, and positional movement).

## 🛠️ Technologies Used
* **C (C99/C11)**
* **SDL2** (Simple DirectMedia Layer) for displaying the pixel buffer and capturing input.
* **pthreads** for parallel processing.

## ⚙️ Compilation & Running (Linux)

### Prerequisites
Make sure you have a C compiler and the SDL2 development libraries installed:
```bash
sudo apt-get update
sudo apt-get install gcc libsdl2-dev
