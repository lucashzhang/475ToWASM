# WASM 475

This is a continuation of the work done in Mike Reed's Fall 2021 COMP 475 class at UNC.

The purpose of this little project is to compile the C++ code written for the assignments into web assembly. This will allow for the code to run and be demonstrated in a browser environment. If things go plan, the code should run at near native speeds.

## Scan Convertor

This code uses a special scan converter that is not the one taught in class. It is an expanded version of a concept that Professor Reed mentioned in the class. Instead of storing an array of edges, it uses a large array of points/dots to represent the outline of the shape needed. While this may be more memory intensive, it avoids the need to constantly resort the edges as well as being more friendly to multithreading.

The extension of the algorithm is the change of the sorting algorithm. The new method originally required a very large quicksort on a very large array of dots, but this was changed into a bucket sort. By shifting to an array of buckets, each point placed on it's y-index, a series of much smaller sorts (generally 2 points per bucket) can sort much faster in near linear time. This is my "improvement" to the scan converter.

## Code Credits

A lot of the boilerplate code in the class was provided by Professor Reed. This means that a large portion of it is still his. While a lot of the structure has been changed, any code that starts with a capital G is his.

## TODOS
1. Connect the WebAssembly to the JavaScript. I'm pretty sure that the code is writing into an UInt8_t array correctly on the C++/wasm side, but I need to send it to the HTML canvas. So far, only JavaScript seems to be viable for setting the buffer into ImageData.

2. Try Multithreading. A big benefit of the atypical Scan Convertor algorithm is it's ability to multithread. Since all points are bucketed by y-index, they can be split as such and different parts of the shape can be rendered in parallel with little worry of race conditions. Emscripten implemented pseudo-multithreading using pthreads and Web Workers, so it's worth a try.