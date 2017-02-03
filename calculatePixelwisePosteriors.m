function [ probabilityMapForeground, probabilityMapBackground ] = calculatePixelwisePosteriors(im, foregroundMap, numberOfBins)
%generateForegroundProbabilityMap Generated probability map
%   The function generates a probability map for each pixel in the input
%   image indicating whether it belongs to the foreground or background

numberOfBinsDiv = 256 / numberOfBins;

% Create Color Histogram
histForeground = zeros(numberOfBins, numberOfBins, numberOfBins, 'double');
histBackground = zeros(numberOfBins, numberOfBins, numberOfBins, 'double');
Mf = 0;

% Create 3D Histogram
for i = 1 : size(im, 1)
    for j = 1 : size(im, 2)
        red = double(im(i, j, 1));
        green = double(im(i, j, 2));
        blue = double(im(i, j, 3));

        % Check if current pixel lies in foreground or background
        if foregroundMap(i, j)
            Mf = Mf + 1;
            % Foreground
            currentValue = histForeground(floor(red/numberOfBinsDiv) + 1, floor(green/numberOfBinsDiv) + 1, floor(blue/numberOfBinsDiv) + 1);
            histForeground(floor(red/numberOfBinsDiv) + 1, floor(green/numberOfBinsDiv) + 1, floor(blue/numberOfBinsDiv) + 1) = currentValue + 1;
        else
            % Background
            currentValue = histBackground(floor(red/numberOfBinsDiv) + 1, floor(green/numberOfBinsDiv) + 1, floor(blue/numberOfBinsDiv) + 1);
            histBackground(floor(red/numberOfBinsDiv) + 1, floor(green/numberOfBinsDiv) + 1, floor(blue/numberOfBinsDiv) + 1) = currentValue + 1;
        end
    end
end

% Normalize both histograms
histForeground = histForeground ./ sum(histForeground(:));
histBackground = histBackground  ./ sum(histBackground(:));

% % Generate pixel probabilities
imageArea = size(im, 1) * size(im, 2);
Mb = (imageArea - Mf) / imageArea; % Probility of background
Mf = Mf / imageArea;

probabilityMapForeground = zeros(size(im, 1), size(im, 2));
probabilityMapBackground = zeros(size(im, 1), size(im, 2));

for i = 1 : size(im, 1)
    for j = 1 : size(im, 2)
        red = double(im(i, j, 1));
        green = double(im(i, j, 2));
        blue = double(im(i, j, 3));

        foregroundProb = histForeground(floor(red/numberOfBinsDiv) + 1, floor(green/numberOfBinsDiv) + 1, floor(blue/numberOfBinsDiv) + 1);
        backgroundProb = histBackground(floor(red/numberOfBinsDiv) + 1, floor(green/numberOfBinsDiv) + 1, floor(blue/numberOfBinsDiv) + 1);
        
        probabilityMapForeground(i, j) = (foregroundProb * Mf) / ((foregroundProb * Mf) + (backgroundProb * Mb));
        probabilityMapBackground(i, j) = (backgroundProb * Mb) / ((foregroundProb * Mf) + (backgroundProb * Mb));
    end
end

end

