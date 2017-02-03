clear all;
close all;

global finished;
global registrationDone;
global numberOfBins;
finished = false;
registrationDone = false;
numberOfBins = 32;

% Declare smooth heaviside and dirac delta step fuctions
smoothHeavisideStepFun = @(x, b) (1 / pi) * (-atan(b * x) + (pi / 2));
smoothDiracDeltaStepFun = @(x, b) (-b / pi) * (1 ./ (1 + (b * x).^2));

% Initialize the system (Omega represents the bounding box of the object)
[phi, p] = init();
sigmaSq = 50; % sigma = sqrt(50)
laplacianKernel = fspecial('laplacian');
bandReach = 3; % b in the above inline functions

diracPhi = dirac(phi);
% warpedIm = imwarp_same(heavisidePhi, affineWarp);
% croppedProbMask = imcrop(warpedIm, omega);

% Load webcam
cam = webcam(1);
frameNum = 1;

% Setup figure
fig = figure;
set(fig, 'KeyPressFcn', @figInputEvent);

% Start camera loop
while ~finished
    % Acquire a single image.
    im = snapshot(cam);
    
    % Convert image to YUV color format
    %im = rgb2hsv(im);

    if registrationDone
        % Calculate kernels
        heavisidePhi = smoothHeavisideStepFun(phi, bandReach); %heaviside(phi);
        diracPhi = smoothDiracDeltaStepFun(phi, bandReach); %dirac(phi);
        lapPhi = imfilter(phi, laplacianKernel);
        [gradMagPhi, gradDirPhi] = imgradient(phi, 'central'); % Returns grad mag and dir computed via central differences
        
        %%%%%%%%%%%% Segmentation %%%%%%%%%%%%
        % Calculate Pf and Pb
        [Pf, Pb] = calculatePixelwisePosteriors(im, heavisidePhi, numberOfBins);
        
        % Equation 12
        finalGradient = (diracPhi .* (Pf - Pb)) ./ ((Pf .* heavisidePhi) + (Pb .* (1 - heavisidePhi))); % Equation 20
        finalGradient = finalGradient - (1 / sigmaSq) .* (lapPhi - gradDirPhi); % gradMagPhi ./ norm(gradMagPhi) = gradientDir
        
        % Optimize for segmentation by following steepest ascent
        finalGradient(isnan(finalGradient)) = 0;
        phi = phi + finalGradient; % No learning rate
        
        % Display the probability map
        subplot(1, 2, 1);
        imshow(im);
        subplot(1, 2, 2);
        imagesc(phi);
        fprintf('%d\n', frameNum);
        frameNum = frameNum + 1;

        %%%%%%%%%%%% Registration / Tracking %%%%%%%%%%%%
        %J = diracPhi * 
        
    else
        % Overlay the image for registration
        %fusedIm = imfuse(im, diracPhi, 'method', 'blend');

        % Display image
        %im = imfuse(im, diracPhi, 'blend','Scaling','independent');
        imshow(im);
        title('Image');
    end
end

clear cam;