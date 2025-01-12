// bitmapManager.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_NAME_SIZE 100

typedef struct
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;

} Pixel;

typedef struct
{
	char file_name[MAX_FILE_NAME_SIZE];
	unsigned int height;
	unsigned int width;
	unsigned int size;
	Pixel **pixels;

} RGB_Image;

// Main function Prototypes

void print_information_image(void);
void save_copy_image(void);
void change_luminosity_image(void);
void remove_channel_image(void);
void invert_image(void);
void quantize_image(void);
void flip_horizontal_image(void);
void crop_image(void);

// FILE Functions
int load_image(RGB_Image *);
int save_image(RGB_Image);

// FREE function
void free_pixels(RGB_Image);

// REALLOC FUNCTION
void re_allocate_pixels(RGB_Image *, int, int);

// PIXEL FUNCTIONS
void invert_pixels(Pixel **, int, int);
void quantize_pixels(Pixel **, int, int, int);
void remove_red_pixels(Pixel **, int, int);
void remove_green_pixels(Pixel **, int, int);
void remove_blue_pixels(Pixel **, int, int);
void change_luminosity_pixels(Pixel **, int, int, int);
void flip_horizontal_pixels(Pixel **, int, int);

void removeBmp(char *);

void rotate_image(RGB_Image *image, int angle)
{
	if (angle % 90 != 0 || angle < 0 || angle > 270)
	{
		printf("Invalid rotation angle! Use 90, 180, or 270.\n");
		return;
	}

	if (angle == 180)
	{
		for (int i = 0; i < image->height / 2; ++i)
		{
			for (int j = 0; j < image->width; ++j)
			{
				Pixel temp = image->pixels[i][j];
				image->pixels[i][j] = image->pixels[image->height - 1 - i][image->width - 1 - j];
				image->pixels[image->height - 1 - i][image->width - 1 - j] = temp;
			}
		}
	}
	else
	{
		int new_height = image->width;
		int new_width = image->height;

		Pixel **rotated_pixels = (Pixel **)malloc(new_height * sizeof(Pixel *));
		for (int i = 0; i < new_height; ++i)
			rotated_pixels[i] = (Pixel *)malloc(new_width * sizeof(Pixel));

		for (int i = 0; i < image->height; ++i)
		{
			for (int j = 0; j < image->width; ++j)
			{
				if (angle == 90)
					rotated_pixels[j][image->height - 1 - i] = image->pixels[i][j];
				else if (angle == 270)
					rotated_pixels[image->width - 1 - j][i] = image->pixels[i][j];
			}
		}

		free_pixels(*image);
		image->pixels = rotated_pixels;
		image->height = new_height;
		image->width = new_width;
	}

	printf("\nImage rotated by %d degrees.\n", angle);
}

void rotate()
{
	RGB_Image image;
	int failedToLoad = load_image(&image);

	if (!failedToLoad)
	{
		int angle;
		printf("\n Enter the angle: ");
		scanf("%d", &angle);

		rotate_image(&image, angle);

		// Append the "_with_margin" suffix to the filename
		strcat(image.file_name, "_with_margin");
		save_image(image);
		free_pixels(image);
	}
}

void apply_blur(RGB_Image *image)
{
	int kernel[3][3] = {
		{1, 2, 1},
		{2, 4, 2},
		{1, 2, 1}};

	int kernel_size = 3;
	int kernel_sum = 16;

	Pixel **blurred_pixels = (Pixel **)malloc(image->height * sizeof(Pixel *));
	for (int i = 0; i < image->height; ++i)
		blurred_pixels[i] = (Pixel *)malloc(image->width * sizeof(Pixel));

	for (int i = 1; i < image->height - 1; ++i)
	{
		for (int j = 1; j < image->width - 1; ++j)
		{
			int red = 0, green = 0, blue = 0;

			for (int ki = -1; ki <= 1; ++ki)
			{
				for (int kj = -1; kj <= 1; ++kj)
				{
					red += image->pixels[i + ki][j + kj].red * kernel[ki + 1][kj + 1];
					green += image->pixels[i + ki][j + kj].green * kernel[ki + 1][kj + 1];
					blue += image->pixels[i + ki][j + kj].blue * kernel[ki + 1][kj + 1];
				}
			}

			blurred_pixels[i][j].red = red / kernel_sum;
			blurred_pixels[i][j].green = green / kernel_sum;
			blurred_pixels[i][j].blue = blue / kernel_sum;
		}
	}

	free_pixels(*image);
	image->pixels = blurred_pixels;

	printf("\nImage blurred.\n");
}

void add_blur()
{

	RGB_Image image;
	int failedToLoad = load_image(&image);
	if (!failedToLoad)
	{

		apply_blur(&image);

		// Append the "_with_margin" suffix to the filename
		strcat(image.file_name, "_with_blur");
		save_image(image);
		free_pixels(image);
	}
}

void adjust_brightness(RGB_Image *image, int brightness)
{
	for (int i = 0; i < image->height; ++i)
	{
		for (int j = 0; j < image->width; ++j)
		{
			int newRed = image->pixels[i][j].red + brightness;
			int newGreen = image->pixels[i][j].green + brightness;
			int newBlue = image->pixels[i][j].blue + brightness;

			image->pixels[i][j].red = (newRed > 255) ? 255 : (newRed < 0 ? 0 : newRed);
			image->pixels[i][j].green = (newGreen > 255) ? 255 : (newGreen < 0 ? 0 : newGreen);
			image->pixels[i][j].blue = (newBlue > 255) ? 255 : (newBlue < 0 ? 0 : newBlue);
		}
	}

	printf("\nBrightness adjusted by %d.\n", brightness);
}

void add_brightness()
{
	RGB_Image image;
	int failedToLoad = load_image(&image);

	if (!failedToLoad)
	{
		int intensity;
		printf("\n Enter the brightness intensity: ");
		scanf("%d", &intensity);

		adjust_brightness(&image, intensity);

		// Append the "_with_margin" suffix to the filename
		strcat(image.file_name, "_with_brightness");
		save_image(image);
		free_pixels(image);
	}
}

void hide_text_in_image(RGB_Image *image, const char *text)
{
	int bit_pos = 0, text_len = strlen(text);

	for (int i = 0; i < image->height; ++i)
	{
		for (int j = 0; j < image->width; ++j)
		{
			if (bit_pos >= text_len * 8)
				break;

			image->pixels[i][j].red = (image->pixels[i][j].red & ~1) | ((text[bit_pos / 8] >> (bit_pos % 8)) & 1);
			bit_pos++;
		}
	}
	printf("\nText hidden in the image successfully.\n");
}

void steganography()
{
	RGB_Image image;
	int failedToLoad = load_image(&image);

	if (!failedToLoad)
	{
		printf("Enter the size of the text to hide: ");
		int text_size;
		scanf("%d", &text_size);

		// Allocate memory for the text dynamically
		char *text = (char *)malloc((text_size + 1) * sizeof(char));
		if (!text)
		{
			printf("Memory allocation failed.\n");
			return;
		}

		printf("Enter the text to hide: ");
		scanf(" %[^\n]s", text); // Read entire line

		hide_text_in_image(&image, text);

		// Append the "_with_margin" suffix to the filename
		strcat(image.file_name, "_with_hidden");
		save_image(image);
		free_pixels(image);
	}
}

void retrieve_text_from_image(RGB_Image *image, int text_length)
{
	char *extracted_text = (char *)malloc((text_length + 1) * sizeof(char));
	if (!extracted_text)
	{
		printf("Memory allocation failed.\n");
		return;
	}

	int bit_pos = 0;
	memset(extracted_text, 0, text_length + 1); // Initialize with null characters

	// Extract text from LSB of red pixels
	for (int i = 0; i < image->height && bit_pos < text_length * 8; ++i)
	{
		for (int j = 0; j < image->width && bit_pos < text_length * 8; ++j)
		{
			int bit = image->pixels[i][j].red & 1;
			extracted_text[bit_pos / 8] |= (bit << (bit_pos % 8));
			bit_pos++;
		}
	}

	printf("\nRetrieved Text: %s\n", extracted_text);
	free(extracted_text);
}

void retrieve_hidden_text()
{
	RGB_Image image;
	int failedToLoad = load_image(&image);

	if (!failedToLoad)
	{
		int text_length;

		printf("Enter the length of the hidden text: ");
		scanf("%d", &text_length);

		retrieve_text_from_image(&image, text_length);

		free_pixels(image);
	}
}

int main()
{
	printf("\n\n ******************* Bitmap Manager *******************\n");
	int choice = 0;
	while (choice != -1)
	{
		printf("\n\n");
		printf("\n\t\t MAIN MENU");
		printf("\n\t Please press enter 0-8, or -1 to Quit");
		printf("\n");
		printf("\n\t 0 - Print image information");
		printf("\n\t 1 - Save copy of image");
		printf("\n\t 2 - Change luminosity of image");
		printf("\n\t 3 - Remove image channel");
		printf("\n\t 4 - Invert image colors");
		printf("\n\t 5 - Quantize image");
		printf("\n\t 6 - Flip image horizontally");
		printf("\n\t 7 - Crop image <EXPERIMENTAL>");
		printf("\n\t 8 - Hide Text in Images - Steganography");
		printf("\n\t 9 - Retreive Hidden Text in Images");
		printf("\n\t 10 - Move Image");
		printf("\n\t 11 - Add Blur");
		printf("\n\t 12 - Adjust Brightness");
		printf("\n\t -1 - Quit");

		printf("\n\n\t Choice >> ");
		scanf("%d", &choice);
		switch (choice)
		{
		case 0:
			print_information_image();
			break;
		case 1:
			save_copy_image();
			break;
		case 2:
			change_luminosity_image();
			break;
		case 3:
			remove_channel_image();
			break;
		case 4:
			invert_image();
			break;
		case 5:
			quantize_image();
			break;
		case 6:
			flip_horizontal_image();
			break;
		case 7:
			crop_image();
			break;
		case 8:
			steganography(); // Call the function to add white margin
			break;
		case 9:
			retrieve_hidden_text();
			break;
		case 10:
			rotate();
			break;
		case 11:
			add_blur();
			break;
		case 12:
			add_brightness();
			break;
		default:
			continue;
		}
	}
}

int load_image(RGB_Image *image_ptr)
{
	printf("\n Enter file name to load (without .bmp extension): ");
	scanf("%s", image_ptr->file_name);
	strcat(image_ptr->file_name, ".bmp");

	FILE *fptr = fopen(image_ptr->file_name, "rb");
	if (NULL == fptr)
	{
		printf("\n File cannot be opened.");
		return 1;
	}

	fseek(fptr, 2, SEEK_SET); // Skip first 2 bytes
	fread(&(image_ptr->size), 4, 1, fptr);
	fseek(fptr, 12, SEEK_CUR); // Skip next 12 bytes
	fread(&(image_ptr->width), 4, 1, fptr);
	fread(&(image_ptr->height), 4, 1, fptr);
	fseek(fptr, 28, SEEK_CUR); // Skip next 28 bytes

	// Dynamically allocating memory for the Pixel[][] array
	image_ptr->pixels = (Pixel **)malloc((image_ptr->height) * sizeof(Pixel *));
	int i, j;
	for (i = 0; i < (image_ptr->height); ++i)
		image_ptr->pixels[i] = (Pixel *)malloc((image_ptr->width) * sizeof(Pixel));

	// Initializing pixel array to 0
	for (i = 0; i < image_ptr->height; ++i)
		for (j = 0; j < image_ptr->width; ++j)
		{
			image_ptr->pixels[i][j].red = 0;
			image_ptr->pixels[i][j].green = 0;
			image_ptr->pixels[i][j].blue = 0;
		}

	// Reading data into pixel array

	for (i = 0; i < image_ptr->height; ++i)
		for (j = 0; j < image_ptr->width; ++j)
		{
			fread(&(image_ptr->pixels[i][j].blue), 1, 1, fptr);
			fread(&(image_ptr->pixels[i][j].green), 1, 1, fptr);
			fread(&(image_ptr->pixels[i][j].red), 1, 1, fptr);
		}

	fclose(fptr);
	printf("\n Image Loaded.\n\n");

	return 0;
}

int save_image(RGB_Image image)
{

	char fileCopy[MAX_FILE_NAME_SIZE];
	strcpy(fileCopy, image.file_name);
	strcat(fileCopy, ".bmp");
	FILE *fptr = fopen(fileCopy, "wb");
	if (NULL == fptr)
	{
		printf("\n File cannot be saved.");
		return 1;
	}

	int data_size = image.size - 54;
	unsigned char bmp_header[] = {
		0x42, 0x4D,
		image.size, image.size >> 8, image.size >> 16, image.size >> 24,
		0x00, 0x00, 0x00, 0x00,
		0x36, 0x00, 0x00, 0x00,
		0x28, 0x00, 0x00, 0x00,
		image.width, image.width >> 8, image.width >> 16, image.width >> 24,
		image.height, image.height >> 8, image.height >> 16, image.height >> 24,
		0x01, 0x00, 0x18, 0x00,
		0x00, 0x00, 0x00, 0x00,
		data_size, data_size >> 8, data_size >> 16, data_size >> 24,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00};

	fwrite(bmp_header, 1, 54, fptr);
	int i, j;
	for (i = 0; i < image.height; ++i)
		for (j = 0; j < image.width; ++j)
		{
			fwrite(&(image.pixels[i][j].blue), 1, 1, fptr);
			fwrite(&(image.pixels[i][j].green), 1, 1, fptr);
			fwrite(&(image.pixels[i][j].red), 1, 1, fptr);
		}

	fclose(fptr);
	printf("\n Image Saved.\n\n");
	return 0;
}

void change_luminosity_pixels(Pixel **pixels, int height, int width, int luminosity_level)
{
	int i, j;
	for (i = 0; i < height; ++i)
	{
		for (j = 0; j < width; ++j)
		{
			if (pixels[i][j].red + luminosity_level > 255)
				pixels[i][j].red = 255;
			else if (pixels[i][j].red + luminosity_level < 0)
				pixels[i][j].red = 0;
			else
				pixels[i][j].red += luminosity_level;

			if (pixels[i][j].green + luminosity_level > 255)
				pixels[i][j].green = 255;
			else if (pixels[i][j].green + luminosity_level < 0)
				pixels[i][j].green = 0;
			else
				pixels[i][j].green += luminosity_level;

			if (pixels[i][j].blue + luminosity_level > 255)
				pixels[i][j].blue = 255;
			else if (pixels[i][j].blue + luminosity_level < 0)
				pixels[i][j].blue = 0;
			else
				pixels[i][j].blue += luminosity_level;
		}
	}
}

void remove_red_pixels(Pixel **pixels, int height, int width)
{
	int i, j;
	for (i = 0; i < height; ++i)
		for (j = 0; j < width; ++j)
			pixels[i][j].red = 0;
}

void remove_green_pixels(Pixel **pixels, int height, int width)
{
	int i, j;
	for (i = 0; i < height; ++i)
		for (j = 0; j < width; ++j)
			pixels[i][j].green = 0;
}
void remove_blue_pixels(Pixel **pixels, int height, int width)
{
	int i, j;
	for (i = 0; i < height; ++i)
		for (j = 0; j < width; ++j)
			pixels[i][j].blue = 0;
}

void invert_pixels(Pixel **pixels, int height, int width)
{
	int i, j;
	for (i = 0; i < height; ++i)
		for (j = 0; j < width; ++j)
		{
			pixels[i][j].red ^= 0xFF;
			pixels[i][j].green ^= 0xFF;
			pixels[i][j].blue ^= 0xFF;
		}
}

void quantize_pixels(Pixel **pixels, int height, int width, int quantization_level)
{

	int i, j;
	unsigned int bit_mask = 0;
	for (i = 0; i < quantization_level - 1; ++i)
	{
		bit_mask++;
		bit_mask = bit_mask << 1;
	}
	bit_mask++;

	quantization_level = bit_mask;

	for (i = 0; i < height; ++i)
		for (j = 0; j < width; ++j)
		{
			pixels[i][j].red &= quantization_level;
			pixels[i][j].green &= quantization_level;
			pixels[i][j].blue &= quantization_level;
		}
}

void flip_horizontal_pixels(Pixel **pixels, int height, int width)
{
	Pixel **temp = (Pixel **)malloc(height * sizeof(Pixel *));
	int i;
	for (i = 0; i < height; ++i)
		temp[i] = (Pixel *)malloc(width * sizeof(Pixel));

	int j, k;
	for (i = 0; i < height; ++i)
		for (j = 0, k = width - 1; j < width; ++j, --k)
			temp[i][j] = pixels[i][k];

	for (i = 0; i < height; ++i)
	{
		pixels[i] = temp[i];
		temp[i] = NULL;
	}
}

void re_allocate_pixels(RGB_Image *image_ptr, int new_height, int new_width)
{
	int pre_height = image_ptr->height;
	int pre_width = image_ptr->width;

	// Allocate memory for new pixel pointers
	Pixel **temp = (Pixel **)malloc(new_height * sizeof(Pixel *));
	if (!temp)
	{
		printf("Memory allocation failed.\n");
		exit(1);
	}

	// Allocate memory for each row and initialize to 0
	for (int i = 0; i < new_height; ++i)
	{
		temp[i] = (Pixel *)calloc(new_width, sizeof(Pixel));
		if (!temp[i])
		{
			printf("Memory allocation failed.\n");
			exit(1);
		}
	}

	// Copy previous pixel data into the new array
	for (int i = 0; i < pre_height; ++i)
	{
		for (int j = 0; j < pre_width; ++j)
		{
			if (i < new_height && j < new_width)
			{
				temp[i][j] = image_ptr->pixels[i][j];
			}
		}
	}

	// Free old pixels
	for (int i = 0; i < pre_height; ++i)
	{
		free(image_ptr->pixels[i]);
	}
	free(image_ptr->pixels);

	// Update the image structure
	image_ptr->pixels = temp;
	image_ptr->height = new_height;
	image_ptr->width = new_width;
	image_ptr->size = new_height * new_width * sizeof(Pixel) + 54;
}

void crop_image()
{
	RGB_Image image;
	int failedToLoad = load_image(&image);
	removeBmp(image.file_name);
	int sizes[] = {100, 200, 300, 400};

	if (!failedToLoad)
	{
		int choice = 0;
		printf("\n Enter the size to crop (1-4):  <EXPERIMENTAL | UNSTABLE>\n"
			   "\n 1. 100x100"
			   "\n 2. 200x200"
			   "\n 3. 300x300"
			   "\n 4. 400x400\n > ");

		scanf("%d", &choice);
		switch (choice)
		{
		case 1:
			re_allocate_pixels(&image, sizes[0], sizes[0]);
			strcat(image.file_name, "_cropped_100");
			printf("\n Image cropped.\n");
			save_image(image);
			break;

		case 2:
			re_allocate_pixels(&image, sizes[1], sizes[1]);
			strcat(image.file_name, "_cropped_200");
			printf("\n Image cropped.\n");
			save_image(image);
			break;

		case 3:
			re_allocate_pixels(&image, sizes[2], sizes[2]);
			strcat(image.file_name, "_cropped_300");
			printf("\n Image cropped.\n");
			save_image(image);
			break;

		case 4:
			re_allocate_pixels(&image, sizes[3], sizes[3]);
			strcat(image.file_name, "_cropped_400");
			printf("\n Image cropped.\n");
			save_image(image);
			break;
		default:
			printf("\n Invalid choice.\n");
			return;
		}
		free_pixels(image);
	}
}
void flip_horizontal_image()
{
	RGB_Image image;
	int failedToLoad = load_image(&image);
	removeBmp(image.file_name);

	if (!failedToLoad)
	{
		flip_horizontal_pixels(image.pixels, image.height, image.width);
		strcat(image.file_name, "_flipped_horizontally");
		printf("\n Image flipped horizontally.\n\n");
		save_image(image);
		free_pixels(image);
	}
}

void quantize_image()
{
	RGB_Image image;
	int failedToLoad = load_image(&image);
	removeBmp(image.file_name);

	int quantization_level = -1;
	while (quantization_level < 0 || quantization_level > 7)
	{
		printf("\n Enter the quantization level (0-7): ");
		scanf("%d", &quantization_level);
	}

	if (!failedToLoad)
	{
		quantize_pixels(image.pixels, image.height, image.width, quantization_level);

		char temp[30];
		sprintf(temp, "_quantize_%d", quantization_level);
		strcat(image.file_name, temp);
		printf("\n Image quantized by a level of %d\n", quantization_level);
		save_image(image);
		free_pixels(image);
	}
}

void invert_image()
{
	RGB_Image image;
	int failedToLoad = load_image(&image);
	removeBmp(image.file_name);

	if (!failedToLoad)
	{
		invert_pixels(image.pixels, image.height, image.width);
		strcat(image.file_name, "_inverted");
		printf("\n Image inverted.\n\n");
		save_image(image);
		free_pixels(image);
	}
}

void remove_channel_image()
{
	RGB_Image image;
	int failedToLoad = load_image(&image);
	removeBmp(image.file_name);

	if (!failedToLoad)
	{
		int choice = 0;
		printf("\n Choose which channel to remove (1-3):\n"
			   "\n 1. Red"
			   "\n 2. Green"
			   "\n 3. Blue\n > ");

		scanf("%d", &choice);
		switch (choice)
		{
		case 1:
			remove_red_pixels(image.pixels, image.height, image.width);
			strcat(image.file_name, "_red_channel_removed");
			printf("\n Red channel removed.\n");
			save_image(image);
			break;

		case 2:
			remove_green_pixels(image.pixels, image.height, image.width);
			strcat(image.file_name, "_green_channel_removed");
			printf("\n Green channel removed.\n");
			save_image(image);
			break;

		case 3:
			remove_blue_pixels(image.pixels, image.height, image.width);
			strcat(image.file_name, "_blue_channel_removed");
			printf("\n Blue channel removed.\n");
			save_image(image);
			break;
		default:
			printf("\n Invalid choice.\n");
			return;
		}
		free_pixels(image);
	}
}

void change_luminosity_image()
{
	RGB_Image image;
	int failedToLoad = load_image(&image);
	removeBmp(image.file_name);
	int luminosity_level = 0;
	printf("\n Enter the luminosity level: ");
	scanf("%d", &luminosity_level);

	if (!failedToLoad)
	{
		change_luminosity_pixels(image.pixels, image.height, image.width, luminosity_level);

		char temp[30];
		sprintf(temp, "_luminosity_%d", luminosity_level);
		strcat(image.file_name, temp);
		printf("\n Image luminosity changed by a level of %d\n", luminosity_level);
		save_image(image);
		free_pixels(image);
	}
}

void save_copy_image()
{
	RGB_Image image;
	int failedToLoad = load_image(&image);
	removeBmp(image.file_name);

	if (!failedToLoad)
	{
		strcat(image.file_name, "_copy");
		printf("\n Image Copied.\n\n");
		save_image(image);
		free_pixels(image);
	}
}

void print_information_image()
{
	RGB_Image image;
	int failedToLoad = load_image(&image);
	if (!failedToLoad)
	{
		printf("\n File name: %s"
			   "\n Image height: %d"
			   "\n Image Width: %d"
			   "\n Image size: %lu",
			   image.file_name, image.height, image.width, image.size);
	}

	free_pixels(image);
}

void free_pixels(RGB_Image image_ptr)
{
	int i;
	for (i = 0; i < image_ptr.height; ++i)
		free(image_ptr.pixels[i]);
}

void removeBmp(char *withBmp)
{
	withBmp[strlen(withBmp) - 4] = '\0';
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
