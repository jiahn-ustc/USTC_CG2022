## 计算机图形学第二次实验报告

> PB19051035周佳豪

### 实验要求

运用Radial basis functions算法实现图像的变形，并解决图像的白色条纹问题。

> Input：一个图像，以及源点与目标点的坐标
>
> output：变形后的图形

### 实验过程

* Radisal basis functions:

  $ f(x) = \sum_{i=1}^n a_ib_i(x) $

  $ b_i(x) = \frac {1}{|x-p_i|^2+d} $

  其中$d$为常数，$$ p_i $$为约束点，$a_i$为变量。

  通过求解以下方程组可得$a_i,i=1\cdots n$

  $$ f(p_i)=q_i-p_i ,i=1 \cdots n$$,需要说明的一点是这个方程与实验文档不同，最终对所有点求得的f映射加上原来点的坐标即为图形变换后的坐标，即f求的是点的增量。

* 代码实现思路

  * 图像变换
    1. 根据起始点的坐标，通过映射b映射成一个矩阵p，该矩阵为n×n矩阵（n表示的是约束点的个数），p~i,j~的值$\frac{1}{|p_i-p_j|^2+d}$,然后求解线性方程组pA = pdst-psrc得n×2的矩阵A。
    2. 对im图片的每个点，通过映射b映射成一个矩阵temp，该矩阵为(h×w)×n矩阵，temp~i,j~的值为$\frac 1{|m_i-p_j|^2+d}$,m~i~表示的是图片的第i个点(图片总共有h×w)个点。
    3. 根据temp*A=result，即可得原图片每个节点的增量，最后即可得到新图片（注意新坐标可能不是整数或者越界，这里需要四舍五入或将越界的坐标删去）。
    4. 值得注意的是读取的矩阵是原图片的转置，即psrc和pdst的坐标是正确的，但根据for循环读取的图片坐标是转置后的，这里需要注意一下。
  * 白点去除
    * 白点即原图片经过线性变换后未映射到所有坐标，即新图片的某一点可能被原图片的两至多个点映射，一个点映射，没有点映射。白点即为未被映射的点。
    * 去除方法：对白点周围非白点的颜色取平均，并将其值赋给非白点。我这里取的是白点周围的8个邻居（左上，上，右上，左，右，左下，下，右下）。

### 实验结果

* d=10

  ![image-20220305192536473](C:\Users\86130\AppData\Roaming\Typora\typora-user-images\image-20220305192536473.png)

  ![image-20220305192603651](C:\Users\86130\AppData\Roaming\Typora\typora-user-images\image-20220305192603651.png)

  ![image-20220305192917908](C:\Users\86130\AppData\Roaming\Typora\typora-user-images\image-20220305192917908.png)

* d=1000

  ![image-20220305192707894](C:\Users\86130\AppData\Roaming\Typora\typora-user-images\image-20220305192707894.png)

  ![image-20220305192820897](C:\Users\86130\AppData\Roaming\Typora\typora-user-images\image-20220305192820897.png)

### 实验总结

* Radial basis functions 算法可以很好的实现图像的变形。
* 通过对实验结果的初步思考，可发现d越小，对远距离的像素影响越小，d越大，对远距离的像素影响越大。

