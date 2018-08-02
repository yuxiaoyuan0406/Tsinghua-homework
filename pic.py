from PIL import Image

filename = input('input file:')

im = Image.open(filename)#.convert('L')  # 打开图片，并处理成灰度图
x, y = im.size
im.resize((x//2, y//2))
im.save('new-' + filename, 'jpeg')
