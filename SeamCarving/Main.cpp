#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <array>
#include <vector>
#include <functional>
#include <thread>
#include <codecvt>

#include "Shader.h"
#include "Vao.h"

typedef unsigned char byte;
typedef long long ll;

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

class Image
{
public:

	std::vector<std::vector<std::array<byte, 4>>> data;
	std::vector<std::vector<byte>> mask;
	int width, height, channels;

	Image(const char* file)
	{
		byte* d = stbi_load(file, &width, &height, &channels, 0);
		if (d == NULL)
		{
			printf("Error loading image from file %s", file);
			return;
		}
		data.assign(height, std::vector<std::array<byte, 4>>(width));
		mask.assign(height, std::vector<byte>(width));
		byte* it = d;
		for (int i = 0; i < height; ++i)
		{
			for (int j = 0; j < width; ++j)
			{
				for (int k = 0; k < channels; ++k)
				{
					data[i][j][k] = *it;
					it++;
				}
			}
		}
		stbi_image_free(d);
	}

	std::vector<byte> to_stream()
	{
		std::vector<byte> stream(width * height * channels);
		int ind = 0;
		for (int i = 0; i < height; ++i)
		{
			for (int j = 0; j < width; j++)
			{
				for (int k = 0; k < channels; ++k)
				{
					stream[ind] = data[i][j][k];
					ind++;
				}
			}
		}
		return stream;
	}

	std::vector<byte> mask_stream()
	{
		std::vector<byte> stream(width * height * channels);
		int ind = 0;
		for (int i = 0; i < height; ++i)
		{
			for (int j = 0; j < width; j++)
			{
				for (int k = 0; k < channels; ++k)
				{
					stream[ind] = mask[i][j];
					ind++;
				}
			}
		}
		return stream;
	}

	void export_jpg(const char* file, int quality)
	{
		stbi_write_jpg(file, width, height, channels, &(to_stream()[0]), quality);
	}

	void export_png(const char* file)
	{
		stbi_write_png(file, width, height, channels, &(to_stream()[0]), width * channels);
	}
};

std::vector<int> carve(Image& img, std::function<ll(std::array<byte, 4>, std::array<byte, 4>)> cost)
{
	std::vector<std::vector<ll>> dp(img.height, std::vector<ll>(img.width, 1e16));
	std::vector<std::vector<int>> last(img.height, std::vector<int>(img.width));

	for (int i = 0; i < img.width; ++i) dp[0][i] = 0;
	for (int i = 1; i < img.height; ++i)
	{
		for (int j = 0; j < img.width; ++j)
		{
			for (int k = std::max(0, j - 1); k <= std::min(img.width - 1, j + 1); ++k)
			{
				ll c = cost(img.data[i - 1][k], img.data[i][j]);
				if (img.mask[i][j] == 0) c += 1e6;
				if (img.mask[i][j] == 255) c += 1e12;
				if (c + dp[i - 1][k] < dp[i][j])
				{
					dp[i][j] = c + dp[i - 1][k];
					last[i][j] = k;
				}
			}
		}
	}

	int p = 0;
	for (int i = 0; i < img.width; ++i)
	{
		if (dp[img.height - 1][i] < dp[img.height - 1][p]) p = i;
	}

	std::vector<int> removed(img.height);
	for (int i = img.height - 1; i >= 0; --i)
	{
		removed[i] = p;
		img.data[i].erase(img.data[i].begin() + p);
		img.mask[i].erase(img.mask[i].begin() + p);
		p = last[i][p];
	}

	img.width--;

	return removed;
}

ll rgb_cost(std::array<byte, 4> a, std::array<byte, 4> b)
{
	ll dr = abs(a[0] - b[0]);
	ll dg = abs(a[1] - b[1]);
	ll db = abs(a[2] - b[2]);
	return dr + dg + db;
}

float cursor_size = 100;
void scroll_callback(GLFWwindow* window, double x, double y)
{
	cursor_size += y * 8;
	if (cursor_size < 8) cursor_size = 8;
}


#include <iostream>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

std::string getFile()
{
	wchar_t filename[MAX_PATH] = {};

	OPENFILENAME ofn;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	ofn.lpstrFilter = L"JPG\0*.jpg\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Select an Image!";
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameW(&ofn))
	{
		std::wcout << L"You chose the file \"" << filename << L"\"\n";
		std::string file = ws2s(std::wstring(filename));
		return file;
	}
	else
	{
		return "";
	}
}

std::string saveFile()
{
	wchar_t filename[MAX_PATH] = {};

	OPENFILENAME ofn;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	ofn.lpstrFilter = L"JPG\0*.jpg\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Select an Image!";
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

	if (GetSaveFileNameW(&ofn))
	{
		std::wcout << L"You chose the file \"" << filename << L"\"\n";
		std::string file = ws2s(std::wstring(filename));
		return file;
	}
	else
	{
		return "";
	}
}

int main()
{
	// IMPORTANT! OTHERWISE FILE_DIALOG WILL BE BLURRY!
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	std::string ifile = getFile();

	int width, height, channels;
	Image img(ifile.c_str());

	GLFWwindow* window = glfwCreateWindow(img.width, img.height, ifile.c_str(), NULL, NULL);
	glfwMakeContextCurrent(window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glViewport(0, 0, img.width, img.height);

	glfwSetScrollCallback(window, scroll_callback);

	//for (int i = 0; i < 600; ++i)
	//{
	//	carve(img, rgb_cost);
	//}
	//img.export_jpg("lake_c2.jpg", 100);

	Program shader = Program::load("shader");
	shader.use();


	Vao rect = Vao::createRect();
	Vao circ = Vao::createCircle(128);

	glBindVertexArray(rect.getId());

	int i = 0;
	int mode = 255;
	bool modeChange = false;

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glViewport(0, 0, img.width, img.height);

		shader.setUniformInt("img", 0);
		shader.setUniformInt("msk", 1);

		glActiveTexture(GL_TEXTURE0);
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// NOT CALLING THIS FUNCTION CAUSES CRASH! (because opengl seems to expect 4 color channels?)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, &(img.to_stream()[0]));

		glActiveTexture(GL_TEXTURE1);
		GLuint msk;
		glGenTextures(1, &msk);
		glBindTexture(GL_TEXTURE_2D, msk);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, &(img.mask_stream()[0]));

		shader.setUniformFloat("mode", 0);
		glBindVertexArray(rect.getId());
		glDrawArrays(GL_TRIANGLES, 0, 6);


		double mx, my;
		glfwGetCursorPos(window, &mx, &my);
		my = img.height - my;
		glViewport(mx - cursor_size/2, my - cursor_size/2, cursor_size, cursor_size);

		shader.setUniformFloat("mode", mode);
		glBindVertexArray(circ.getId());
		glDrawArrays(GL_LINE_STRIP, 0, 128);

		my = img.height - my;
		int val = -1;
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
		{
			if (!modeChange)
			{
				if (mode == 255) mode = 1;
				else mode = 255;
				modeChange = true;
			}
		}
		else
		{
			modeChange = false;
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) val = mode;
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) val = 0;

		if (val != -1)
		{
			for (int i = -cursor_size/2; i <= cursor_size/2; ++i)
			{
				for (int j = -cursor_size/2; j <= cursor_size/2; ++j)
				{
					if (i * i + j * j > cursor_size * cursor_size / 4) continue;
					if (mx + j < 0 || mx + j >= img.width) continue;
					if (my + i < 0 || my + i >= img.height) continue;
					img.mask[my + i][mx + j] = val;
				}
			}
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			carve(img, rgb_cost);
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			std::string ofile = saveFile();
			img.export_jpg(ofile.c_str(), 100);
		}

		glfwPollEvents();
		glfwSwapBuffers(window);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &tex);
		glDeleteTextures(1, &msk);
	}
}