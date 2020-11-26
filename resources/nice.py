import json

hashy = {
            9  : "12436720_c.png",
            10 : "6B2D96F_c.png",
            11 : "359289F2_c.png",
            12 : "10E99677_c.png",
            13 : "3F485258_c.png",
            14 : "C1DF883_c.png",
            15 : "3D49A9D5_c.png",
            16 : "6C631877_c.png",
            17 : "1B46C8C_c.png",
            18 : "4020CDFE_c.png",
            19 : "275F399C_c.png",
            22 : "1FAAE88D_c.png",
            23 : "1FAAE88D_c.png",
            24 : "12436720_c.png",
            25 : "6B2D96F_c.png",
            26 : "6B1A233B_c.png",
            27 : "6E3A21B_c.png",
            28 : "359289F2_c.png",
            29 : "1FAAE88D_c.png",
            64 : "41A41EE3_c.png",
            65 : "574B138E_c.png"
         }
nice = []
nice2 = []

with open('bobomb.obj') as fp:
   line = fp.readline()
   i = 0
   while line:
       if line[0] == 'o':
           nice.append(hashy[int(line[23:26])])
           nice2.append(int(line[23:26]))
           i = i+1
       line = fp.readline()
           

print(json.dumps(nice))
print(len(nice))
print(nice2)