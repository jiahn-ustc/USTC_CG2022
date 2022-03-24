//   Copyright © 2021, Renjie Chen @ USTC

#pragma once

#include <cassert>
#include <vector>
#include <string>
#include <sys/stat.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

//    int x,y,n;
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
//    // ... process data if not NULL ...
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    // ... but 'n' will always be the number that it would have been if you said 0
//    stbi_image_free(data)
using namespace std;


class MyImage
{
private:
	std::vector<BYTE> pixels;
	int w, h, comp;

public:
	MyImage() : w(0), h(0), comp(0) {}
	~MyImage() {}

	MyImage(const std::string &filename, int ncomp = 4) : w(0), h(0), comp(0)
	{
		stbi_set_flip_vertically_on_load(true);
		BYTE *data = stbi_load(filename.data(), &w, &h, &comp, ncomp);

		if (data)
		{
			pixels = std::vector<BYTE>(data, data + w * h * comp);
			stbi_image_free(data);
		}
		else
		{
			fprintf(stderr, "failed to load image file %s!\n", filename.c_str());
			struct stat info;
			if (stat(filename.c_str(), &info))
				fprintf(stderr, "file doesn't exist!\n");
		}
	}

	MyImage(BYTE *data, int ww, int hh, int pitch, int ncomp = 3) : w(ww), h(hh), comp(ncomp)
	{
		if (pitch == w * comp)
			pixels = std::vector<BYTE>(data, data + pitch * h);
		else
		{
			pixels.resize(w * comp * h);
			for (int i = 0; i < h; i++)
				std::copy_n(data + pitch * i, pitch, pixels.data() + i * w * comp);
		}
	}

	static int alignment() { return 1; } // OpenGL only supports 1,2,4,8, do not use 8, it is buggy

	inline bool empty() const { return pixels.empty(); }

	inline BYTE *data() { return pixels.data(); }
	inline const BYTE *data() const { return pixels.data(); }
	inline int width() const { return w; }
	inline int height() const { return h; }
	inline int dim() const { return comp; }
	inline int pitch() const { return w * comp; }

	MyImage rescale(int ww, int hh) const
	{
		std::vector<BYTE> data(ww * comp * hh);
		stbir_resize_uint8(pixels.data(), w, h, w * comp,
						   data.data(), ww, hh, ww * comp, comp);

		return MyImage(data.data(), ww, hh, ww * comp, comp);
	}

	MyImage resizeCanvas(int ww, int hh)
	{
		std::vector<BYTE> data(ww * comp * hh, 255);
		for (int i = 0; i < h; i++)
			std::copy_n(pixels.data() + i * w * comp, w * comp, data.data() + i * ww * comp);

		return MyImage(data.data(), ww, hh, ww * comp, comp);
	}

	inline void write(const std::string &filename, bool vflip = true) const
	{
		if (filename.size() < 4 || !_strcmpi(filename.data() + filename.size() - 4, ".png"))
		{
			stbi_write_png(filename.data(), w, h, comp, pixels.data() + (vflip ? w * comp * (h - 1) : 0), w * comp * (vflip ? -1 : 1));
		}
		else
		{
			fprintf(stderr, "only png file format(%s) is supported for writing!\n", filename.c_str());
		}
	}

	inline std::vector<BYTE> bits(int align = 1) const
	{
		const int pitch = (w * comp + align - 1) / align * align;

		std::vector<BYTE> data(pitch * h);
		for (int i = 0; i < h; i++)
			std::copy_n(pixels.data() + i * w * comp, w * comp, data.data() + i * pitch);

		return data;
	}
	//重载(),返回RGB数据
	vector<BYTE> operator()(int i, int j)
	{
		vector<BYTE> data(3);
		for (int k = 0; k < 3; k++)
			data[k] = pixels[(i * w + j) * comp + k];
		return data;
	}
	//灰度图返回一个分量,由于能量图的三个RGB分量是值一样的,所以只返回第一个分量即可作为该点的能量值
	int getGrayValue(int i, int j)
	{
		return int(pixels[(i * w + j) * comp]);
	}
	//根据col，删去图像的一列
	void crop_one_col(vector<int> col)
	{
		// cout<<"begin crop one col"<<endl;
		vector<BYTE> data((w - 1) * h * comp);
		// cout<<"w="<<w<<" h="<<h<<endl;
		int i1 = 0, j1 = 0;
		int i2 = 0, j2 = 0;
		int index = 0;
		while (i1 < h && i2 < h)
		{
			j1 = j2 = 0;
			//依次拷贝，遇到col中的数据就跳过
			while (j1 < w && j2 < w - 1)
			{
				if (j1 == col[index])
				{
					j1++;
				}
				for (int k = 0; k < comp; k++)
				{
					data[comp * (i2 * (w - 1) + j2) + k] = pixels[comp * (i1 * w + j1) + k];
				}
				j2++;
				j1++;
			}
			index++;
			i1++;
			i2++;
		}

		this->pixels = data;
		w = w - 1;

	}
	//根据col，扩充图像的一列
	void expand_one_col(vector<int> col)
	{
		// cout<<"begin expand a col"<<endl;
		vector<BYTE> data((w + 1) * h * comp);

		int i1 = 0, j1 = 0;
		int i2 = 0, j2 = 0;
		int index = 0;
		while (i1 < h && i2 < h)
		{
			j1 = j2 = 0;
			//依次拷贝，遇到col中的数据就重复拷贝一次
			while (j1 < w && j2 < w + 1)
			{
				if (j1 == col[index])
				{
					for (int k = 0; k < comp; k++)
					{
						data[comp * (i2 * (w + 1) + j2) + k] = pixels[comp * (i1 * w + j1) + k];
					}
					j2++;
				}
				for (int k = 0; k < comp; k++)
				{
					data[comp * (i2 * (w + 1) + j2) + k] = pixels[comp * (i1 * w + j1) + k];
				}
				j2++;
				j1++;
			}
			index++;
			i1++;
			i2++;
		}

		this->pixels = data;
		w = w + 1;
	}
	
	//根据line，删去图像的一行
	void crop_one_line(vector<int> line)
	{
		vector<BYTE> data(w * (h - 1) * comp);
		int i1 = 0, i2 = 0;
		int j1 = 0, j2 = 0;
		int index = 0;
		while (j1 < w && j2 < w)
		{
			i1 = i2 = 0;
			//依次拷贝，遇到line中的数据就跳过
			while (i1 < h && i2 < h - 1)
			{
				if (i1 == line[index])
				{
					i1++;
				}
				for (int k = 0; k < comp; k++)
				{
					data[comp * (i2 * w + j2) + k] = pixels[comp * (i1 * w + j1) + k];
				}
				i1++;
				i2++;
			}
			index++;
			j1++;
			j2++;
		}

		h = h - 1;
		this->pixels = data;
	}
	void expand_one_line(vector<int> line)
	{
		vector<BYTE> data(w * (h + 1) * comp);
		int i1 = 0, i2 = 0;
		int j1 = 0, j2 = 0;
		int index = 0;
		while (j1 < w && j2 < w)
		{
			i1 = i2 = 0;
			//依次拷贝，遇到line中的数据就重复拷贝一次
			while (i1 < h && i2 < h + 1)
			{
				if (i1 == line[index])
				{
					for (int k = 0; k < comp; k++)
					{
						data[comp * (i2 * w + j2) + k] = pixels[comp * (i1 * w + j1) + k];
					}
					i2++;
				}
				for (int k = 0; k < comp; k++)
				{
					data[comp * (i2 * w + j2) + k] = pixels[comp * (i1 * w + j1) + k];
				}
				i1++;
				i2++;
			}
			index++;
			j1++;
			j2++;
		}

		h = h + 1;
		this->pixels = data;
	}
	//能量图扩充一列
	void grayHaveVisitedOneCol(vector<int> col)
	{
		vector<BYTE> data((w + 1) * h * comp);
		int i1 = 0, j1 = 0;
		int i2 = 0, j2 = 0;
		int index = 0;
		while (i1 < h && i2 < h)
		{
			j1 = j2 = 0;

			while (j1 < w && j2 < w + 1)
			{
				if (j1 == col[index])
				{
					//执行两次，第一次将该列赋最大值
					//第二次将扩充的一列赋最大值
					for (int m = 0; m < 2; m++)
					{
						for (int k = 0; k < 3; k++)
						{
							data[comp * (i2 * (w + 1) + j2) + k] = 255;
						}
						if (comp == 4)
						{
							data[comp * (i2 * (w + 1) + j2) + 3] = pixels[comp * (i1 * w + j1) + 3];
						}
						j2++;
					}
					j1++;
				}
				else
				{
					for (int k = 0; k < comp; k++)
					{
						data[comp * (i2 * (w + 1) + j2) + k] = pixels[comp * (i1 * w + j1) + k];
					}
					j2++;
					j1++;
				}
			}
			index++;
			i1++;
			i2++;
		}

		this->pixels = data;
		w = w + 1;
	}
	//能量图扩充一行
	void grayHaveVisitedOneLine(vector<int> line)
	{
		vector<BYTE> data(w * (h + 1) * comp);
		int i1 = 0, i2 = 0;
		int j1 = 0, j2 = 0;
		int index = 0;
		while (j1 < w && j2 < w)
		{
			i1 = i2 = 0;
			while (i1 < h && i2 < h + 1)
			{
				if (i1 == line[index])
				{
					//执行两次，第一次将该行赋最大值
					//第二次将扩充的一行赋最大值
					for (int m = 0; m < 2; m++)
					{
						for (int k = 0; k < 3; k++)
						{
							data[comp * (i2 * w + j2) + k] = 255;
						}
						if (comp == 4)
						{
							data[comp * (i2 * w + j2) + 3] = pixels[comp * (i1 * w + j1) + 3];
						}
						i2++;
					}
					i1++;
				}
				else
				{
					for (int k = 0; k < comp; k++)
					{
						data[comp * (i2 * w + j2) + k] = pixels[comp * (i1 * w + j1) + k];
					}
					i1++;
					i2++;
				}
			}
			index++;
			j1++;
			j2++;
		}

		h = h + 1;
		this->pixels = data;
	}
};
