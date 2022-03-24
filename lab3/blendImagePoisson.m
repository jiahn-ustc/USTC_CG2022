function imret = blendImagePoisson(im1, im2, roi, targetPosition)

% input: im1 (background), im2 (foreground), roi (in im2), targetPosition (in im1)

%% TODO: compute blended image

fprintf('roi:\n');
%roi
fprintf('target:\n');
%targetPosition


max_cr= max(roi);
min_cr = min(roi);
max_cr = ceil(max_cr);
min_cr = ceil(min_cr);

roi_x =  min_cr(2);
roi_y = min_cr(1);



min_target = ceil(min(targetPosition));
target_x = min_target(2);
target_y = min_target(1);

roi_col = max_cr(1)-min_cr(1);
roi_line = max_cr(2)-min_cr(2);

%mask为包含该区域的最小长方形
mask = int32(poly2mask(roi(:,1)-min_cr(1),roi(:,2)-min_cr(2),roi_line,roi_col));

%边界赋值为0
mask(1,:) = 0;
mask(end,:) = 0;
mask(:,1) = 0;
mask(:,end) = 0;

%给矩阵中每个在roi中的点编号以便构造稀疏矩阵A以及矩阵b
num = 1;
for i=1:roi_line
    for j=1:roi_col
        if(mask(i,j)==1)
            mask(i,j) = num;
            num=num+1;
        end
    end
end
num = num-1;%num表示内部点的总个数，即需要计算的点的总数
%fprintf('num:%d\n', num);


%构造稀疏矩阵A
%使用三元组<row,col,val>表示稀疏矩阵A
row = zeros(num*5,1);
col = zeros(num*5,1);
val = zeros(num*5,1);
count = 0;%表示矩阵A第几行
numPoints = 0;%记录稀疏矩阵中的非零元素个数
for i=1:roi_line
    for j=1:roi_col
        if(mask(i,j)~=0)%内部节点
            count = count+1;
            numPoints = numPoints+1;
            row(numPoints) = count;
            col(numPoints) = count;
            val(numPoints) = -4;
            if(mask(i+1,j)~=0)
                numPoints = numPoints+1;
                row(numPoints) = count;
                col(numPoints) = mask(i+1,j);
                val(numPoints) = 1;
            end
            if(mask(i-1,j)~=0)
                numPoints = numPoints+1;
                row(numPoints) = count;
                col(numPoints) = mask(i-1,j);
                val(numPoints) = 1;
            end
            if(mask(i,j+1)~=0)
                numPoints = numPoints+1;
                row(numPoints) = count;
                col(numPoints) = mask(i,j+1);
                val(numPoints) = 1;
            end
            if(mask(i,j-1)~=0)
                numPoints = numPoints+1;
                row(numPoints) = count;
                col(numPoints) = mask(i,j-1);
                val(numPoints) = 1;
            end
        end

    end
end
%求得稀疏矩阵A
A= sparse(row(1:numPoints),col(1:numPoints),val(1:numPoints));

dA = decomposition(A);%求解稀疏矩阵A的分解

%矩阵b的创建
b = double(zeros(count,3));
count = 0;%count表示b的行数
for i=1:roi_line
    for j=1:roi_col
        if(mask(i,j)~=0)
            count = count+1;
            if mask(i-1,j)~=0 
                %注意等式右端为1x1x3矩阵，不能直接赋值，需要reshape成1x3矩阵
                b(count,:) = b(count,:)+ reshape(-double(im2( roi_x + i,roi_y + j, :))+double(im2( roi_x + i-1,roi_y + j, :)),[1,3]); 
            else
                b(count,:) = b(count,:)+ reshape( -double( im1( target_x + i-1, target_y + j, :)),[1,3]);
            end
            if mask(i+1,j)~=0           
                b(count,:) = b(count,:)+ reshape(-double(im2( roi_x + i,roi_y + j, :))+double(im2( roi_x + i+1,roi_y + j, :)),[1,3]);
            else
                b(count,:) = b(count,:)+ reshape( -double( im1( target_x + i+1, target_y + j, :)),[1,3]);
            end
            if mask(i,j-1)~=0           
                b(count,:) = b(count,:)+ reshape(-double(im2( roi_x + i,roi_y + j, :))+double(im2( roi_x + i,roi_y + j-1, :)),[1,3]);
            else
                b(count,:) = b(count,:)+ reshape(-double( im1( target_x + i, target_y + j-1, :)),[1,3]);
            end
            if mask(i,j+1)~=0 
                b(count,:) = b(count,:)+ reshape(-double(im2( roi_x + i,roi_y + j, :))+double(im2( roi_x + i,roi_y + j+1, :)),[1,3]);
            else
                b(count,:) = b(count,:)+ reshape( -double( im1( target_x + i, target_y + j+1, :)),[1,3]);
            end
        end
    end
end

%求得f
img = dA\b;

imret = im1;
count = 0;
for i=1:roi_line
    for j=1:roi_col
        if(mask(i,j)~=0)
            count = count+1;
            imret(target_x + i, target_y + j, :) = img(count,:);
        end
    end
end