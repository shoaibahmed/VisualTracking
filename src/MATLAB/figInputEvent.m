function figInputEvent(src, event)
% Print button pressed
disp(event.Key);

% Quit button
if strcmp(event.Key, 'q')
    global finished;
    if ~finished
        finished = true;
    else
        close all;
    end
    
% Register button
elseif strcmp(event.Key, 'r')
    global registrationDone;
    registrationDone = true;
end

end