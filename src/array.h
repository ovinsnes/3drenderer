/*
 *
MIT License

Copyright (c) 2020 Zhou Le

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef ARRAY_H
#define ARRAY_H

////////////////////////////////////////////////////////////////////////////////
/// Macro to push elements to an array
////////////////////////////////////////////////////////////////////////////////
#define array_push(array, value)									\
	do {															\
		(array) = array_hold((array), 1, sizeof(*(array)));			\
		(array)[array_length(array) - 1] = (value);					\
	} while (0);

void* array_hold(void* array, int count, int item_size); // Internal function to hold elements
int array_length(void* array); // Number of array elements
void array_free(void* array);

#endif
