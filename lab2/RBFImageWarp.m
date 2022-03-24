function im2 = RBFImageWarp(im, psrc, pdst)

% input: im, psrc, pdst


%% basic image manipulations
% get image (matrix) size
[h, w, dim] = size(im);


im2 = ones(h, w, dim,'uint8')*255;


numPoints = size(psrc,1); % number of src points
d = 10; 


%% use loop to negate image


A = zeros(numPoints,2);

p = zeros(numPoints);
for i=1:numPoints
        p(i,:)=(1 ./(sum(( psrc(i,:)-psrc ).^2,2) +d))';
end

%求得关键矩阵A
A = p\(pdst-psrc);




temp = zeros(h*w,numPoints);
result = zeros(h*w,2);
c = 1;
for i=1:h
    for j=1:w
        temp(c,:)=(1 ./(sum(( [j,i]-psrc ).^2,2) +d))';
        c = c+1;
    end
end

result = temp * A;

c=1;
newresult = result;
for i=1:h
    for j=1:w
        newresult(c,:) = result(c,:) + [j,i];
        newresult(c,:) = ceil(newresult(c,:));
        %防止越界
        if newresult(c,1)<1
            newresult(c,1) = 1;
        elseif newresult(c,1)>h
            newresult(c,1) = h;
        end
        if newresult(c,2)<1
            newresult(c,2) = 1;
        elseif newresult(c,2)>w
            newresult(c,2) = w;
        end
        c = c+1;
    end
end





c = 1;
for i=1:h
    for j=1:w
        %求得im2
        im2(newresult(c,2),newresult(c,1),:) = im(i,j,:);
        c = c + 1;
    end
end

white = ones(1,1,3).*255;
%去除白点
for i=1:h
    for j=1:w
        if isequal(im2(i,j,:),white)
            sum_num = zeros(1,1,3);
            num = 0;
            intNum = zeros(1,1,3);
            %上
            if (i-1>=1 && ~isequal(im2(i-1,j,:),white))
                sum_num = sum_num + double(im2(i-1,j,:));
                num = num + 1;
            end
            %左
            if (j-1>=1 && ~isequal(im2(i,j-1,:),white))
                sum_num = sum_num + double(im2(i,j-1,:));
                num = num + 1;
            end
            %下
            if(i+1<=h && ~isequal(im2(i+1,j,:),white))
                sum_num = sum_num + double(im2(i+1,j,:));
                num = num + 1;
            end
            %右
            if(j+1<=w && ~isequal(im2(i,j+1,:),white))
                sum_num = sum_num + double(im2(i,j+1,:));
                num = num + 1;
            end
            %左上
            if(i-1>=1 && j-1>=1 && ~isequal(im2(i-1,j-1,:),white))
                sum_num = sum_num + double(im2(i-1,j-1,:));
                num = num + 1;
            end
            %右上
            if(i-1>=1 && j+1<=w && ~isequal(im2(i-1,j+1,:),white))
                sum_num = sum_num + double(im2(i-1,j+1,:));
                num = num + 1; 
            end
            %左下
            if(i+1<=h && j-1>=1 && ~isequal(im2(i+1,j-1,:),white))
                sum_num = sum_num + double(im2(i+1,j-1,:));
                num = num + 1;
            end
            %右下
            if(i+1<=h && j+1<=w && ~isequal(im2(i+1,j+1,:),white)) 
                sum_num = sum_num + double(im2(i+1,j+1,:));
                num = num + 1;
            end
            im2(i,j,:) =  sum_num ./ num;
        end
    end
end









%% TODO: compute warpped image