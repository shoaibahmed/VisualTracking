function [ phi, p ] = init()
%INIT Initialize the complete system
% Constructs a shape kernels and returns the level-set formulation using euclidean distance transform
% Also setups the parameters of affine warp

% Load image contour as a binary image (Level-set zero)
x = imread('template.png');
binaryImage = rgb2gray(x) > 0;

% Find boundary for object (x is horizontal and Y is vertical)
[nonZeroPixelsY, nonZeroPixelsX] = find(binaryImage);
x = min(nonZeroPixelsX);
y = min(nonZeroPixelsY);
w = max(nonZeroPixelsX) - x;
h = max(nonZeroPixelsY) - y;

relaxation = 20; % Ease the boundary by specified number of pixels
omega = [x  - relaxation, y - relaxation, w + (2 * relaxation), h + (2 * relaxation)];
% B = bwtraceboundary(binaryImage, [nonZeroPixelsX(1), nonZeroPixelsY(1)], 'S');
% 
% contour = zeros(size(binaryImage));
% for i = 1 : length(B)
%     contour(B(i, 1), B(i, 2)) = 1;
% end

% Shape kernel
phi = bwdist(binaryImage);
phi = phi - bwdist(1 - binaryImage);
phi = -phi; % As phi is positive inside the shape, and negative outside it

%phi(phi >= 0) = phi(phi >= 0) - 1; % For making the contour zero distance

% Affine warp params (p)
% p = [1 0 0; 
%      0 1 0; 
%      -omega(1) -omega(2) 1];

% If registration is correct, W(x, p) = x where W takes a pixel location in object frame and warps it into the image frame
p = affine2d(); % Creates identity transform
 
end
