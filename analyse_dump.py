file = open("C:/Users/Apoptosis/Documents/My_Files/Git_repos/tryimgui/serialMonitor/build/datarunner/Release/dumpfile", mode='rb')
fileContent = file.read()

data_form = [ord('&'), 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, ord('\n')]
for i in range(len(fileContent)):
    if fileContent[i] != data_form[i % 22]:
        print("wrong packet pos: ", i, fileContent[i], data_form[i % 22])
        break

file.close()