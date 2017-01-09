clear all;
close all;

global finished;
global registrationDone;
global numberOfBins;
finished = false;
registrationDone = false;
numberOfBins = 32;

% Initialize the system (Omega represents the bounding box of the object)
[phi, foregroundProbMask, omega, p] = init();

affineWarp = affine2d(p);
warpedIm = imwarp_same(foregroundProbMask, affineWarp);
croppedProbMask = imcrop(warpedIm, omega);

% % Display image
% imshow(warpedIm);
% title('Warped Image');

% Load webcam
cam = webcam(1);

% Setup figure
fig = figure;
set(fig, 'KeyPressFcn', @figInputEvent);

% Start camera loop
while ~finished
    % Acquire a single image.
    im = snapshot(cam);
    
    % Convert image to YUV color format
    %im = rgb2hsv(im);
    
    % Display image
    imshow(im);
    title('Image');

    % Get foreground rect
    %foregroundRect = getrect;
    %foregroundRect = uint64(foregroundRect);

    if registrationDone
        % Extract and display mask and patch
        %mask = zeros(size(im, 1), size(im, 2));
        %mask(foregroundRect(2):(foregroundRect(2) + foregroundRect(4)), foregroundRect(1):(foregroundRect(1) + foregroundRect(3))) = 1;
        %patch = im(foregroundRect(2):(foregroundRect(2) + foregroundRect(4)), foregroundRect(1):(foregroundRect(1) + foregroundRect(3)), :);

%         figure
%         imshow(mask);
%         title('Mask');
%         figure
%         imshow(patch);
%         title('Foreground Patch');

        % Calculate color histogram of image
        % histIm = histogram(im, numberOfBins); % Histogram for whole image
        % histForeground = histogram(patch, numberOfBins);    % Histogram of mask
        % histBackground = histIm - histForeground;
        
        warpedIm = imwarp_same(im, affineWarp);
        croppedIm = imcrop(warpedIm, omega);

        [probImFG, probImBG] = generateForegroundProbabilityMap(croppedIm, croppedProbMask, numberOfBins);

%         % Display probability map
%         figure
%         imshow(probImFG);
%         title('Foreground Probability Map');
% 
%         figure
%         imshow(probImBG);
%         title('Background Probability Map');
    end
end

clear cam;
