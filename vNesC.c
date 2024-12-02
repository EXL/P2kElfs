// Nes.c
// NES emulator

#include "vNesC.h"

static void serviceRepaints() { }
static void Systemgc() { }
static long SystemcurrentTimeMillis() { return 0; }

static void k() ;
static void f() ;
static void l() ;
static void xxf(int i1, int j1) ;
static void xxd(int i1, int j1) ;
static void j() ;
static void xd(int i1) ;
static boolean bbool() ;
static void m() ;
static int xxb(int i1, int j1) ;
//static void d() ;
static void c0() ;
static void avoid1int(int i1) ;
static void g() ;
static int loadROM(char *s) ;
static void avoid() ;
static void b() ;
static boolean abool() ;
static void h() ;
static int aint1int(int i1) ;
static void i() ;
static void e(int i1, int j1) ;
static void xxxxb(int i1, int j1, int k1, int l1) ;
static void avoid4int(int i1, int j1, int k1, int l1) ;
static void c(int i1, int j1) ;
static void avoid8int(int i1, int j1, int k1, int l1, int i2, int j2, int k2, int l2) ;
static void bvoid2int(int i1, int j1) ;
static int aint2int(int i1, int j1) ;
//static void b(InputStream inputstream) throws IOException
static int bint1int(int i1) ;
static void avoid2int(int i1, int j1) ;
static void c_out();
static void b_out();
static void b_in();
static void c_in();

unsigned char sbuf[sbufsize], *ebuf=sbuf;

static int main_a_int_fld, main_c_int_fld;
static boolean main_b_boolean_fld;
int getWidth, getHeight, screen_length;
static int nn; //temp

static volatile int running;

    static int b_int_static_fld = 128;
    static int Z = 0;
    static int f_int_static_fld = 0;
    static int n_int_static_fld = 0;
    static int Q = 0;
    static int w = 0;
    static int P;
    static int ad;
    static int ab = 0;
    static int S = 0;
    static int R = 0;
    static int o_int_static_fld = 0;
    static int L = 128;
    static int b_int_array1d_static_fld[8];
    static int e_int_static_fld = 0;
    static int H = 0;
//    static const char *a_java_lang_String_array1d_static_fld[] = {
//        "Up", "Down", "Left", "Right", "A", "B", "Start", "Select", "AA", "BB", 
//        "Trigger", "Quick Save", "Quick Load", "FullScreen"
//    };
    static int key[] = {
        50, 56, 52, 54, 49, 51, 
        53, 48, 55, 57, 
        35, 64, 65, 42
    };
   // '2'=10 8=20 4=40 6=80 1=1 3=2 0=8 7=4
    static int O = 0;
    static int g_int_static_fld = 0;
    static int ac = 0;
    static int N = 0;
    static int s_int_static_fld = 0;
    static int v_int_static_fld = 0;
    static int B = 100;
    static int z = 0;
    static boolean q_boolean_static_fld = true;
    static boolean a_boolean_static_fld = false;
    static boolean r_boolean_static_fld = false;
    static boolean i_boolean_static_fld = false;
    static int aa = -1;
    static int I = 0;
    static boolean e_boolean_static_fld = false;
    static int F = 0;
    static int k_int_static_fld = 0;
    static int a_int_static_fld = 0;
    static int X = 0;
    static boolean o_boolean_static_fld = false;
    static char *a_java_lang_String_static_fld = null;
    static int y = 0;
    static int D = 0;
    static int c_int_static_fld = 0;
    static byte *vram=0;//free?
    static boolean f_boolean_static_fld = false;
    static boolean g_boolean_static_fld = false;
    static int T = 0;
    static int W = 0;
    static int U = 0;
    static int d_int_static_fld = 0;
    static int m_int_static_fld = 0;
    static long c_long_static_fld;
    static long b_long_static_fld;
    static int i_int_static_fld = 0;
    static int u_int_static_fld = 0;
    static int K = -1;
    static int t_int_static_fld = -1;
    static boolean t_boolean_static_fld = true;
    static ushort j_int_array1d_static_fld[32];
    static ushort g_int_array1d_static_fld[64];
    static int e_int_array1d_static_fld[32];
    static boolean p_boolean_static_fld = false;
//    static NESx65 main;
    static int x = 1;
    static boolean b_boolean_static_fld = false;
    static int i_int_array1d_static_fld[12];
    static int l_int_array1d_static_fld[8];
    static ushort *a_char_array1d_static_fld=0; //free!
    static int j_int_static_fld = 0;
    //static byte a_byte_array1d_static_fld[];
    static int q_int_static_fld = 0;
    static int E = 0;
    static int G = 0;
    static int J = 1;
    static int Y = 0;
    static boolean u_boolean_static_fld = false;
    static int A = 5;
    static int d_int_array1d_static_fld[8];//=0; //free?????
    static boolean v_boolean_static_fld = false;
    static boolean h_boolean_static_fld = false;
    static char *b_java_lang_String_static_fld;
    static boolean m_boolean_static_fld = false;
    static boolean s_boolean_static_fld = false;
    static boolean d_boolean_static_fld = false;
//    static boolean k_boolean_static_fld = false;
    static int V = 4;
    static int p_int_static_fld = 0;
    static byte d_byte_array1d_static_fld[256];
    static int C = 0;
    static boolean n_boolean_static_fld = false;
    static long a_long_static_fld = 0L;
    static byte *b_byte_array1d_static_fld=0; //free
    static const ushort a_short_array1d_static_fld[] = {
        0, 17, 35, 52, 70, 87, 105, 122, 139, 156, 
        174, 191, 208, 225, 242, 259, 276, 292, 309, 326, 
        342, 358, 375, 391, 407, 423, 438, 454, 469, 485, 
        500, 515, 530, 545, 559, 574, 588, 602, 616, 629, 
        643, 656, 669, 682, 695, 707, 719, 731, 743, 755, 
        766, 777, 788, 799, 809, 819, 829, 839, 848, 857, 
        866, 875, 883, 891, 899, 906, 914, 921, 927, 934, 
        940, 946, 951, 956, 961, 966, 970, 974, 978, 982, 
        985, 988, 990, 993, 995, 996, 998, 999, 999, 1000, 
        1000
    };
    static boolean c_boolean_static_fld = false;
    static boolean l_boolean_static_fld = false;
    static boolean j_boolean_static_fld = false;
    ushort *screens=0; //free screen!!!
    static int h_int_static_fld;
    static int r_int_static_fld;
    static int c_int_array1d_static_fld[260];
    static int h_int_array1d_static_fld[276];
    static int f_int_array1d_static_fld[276];
    static boolean b_boolean_array1d_static_fld[260];
    static boolean a_boolean_array1d_static_fld[276];
    static int l_int_static_fld = -1;
    static int M = 0;
    static int ram=0, ramsize=0;
    static int keyindex=-1;

    static void k() {
        xxxxb(
            (m_int_static_fld << 5) + (Z & 0x1f),
            (m_int_static_fld << 5) + (f_int_static_fld & 0x1f),
            (m_int_static_fld << 5) + (n_int_static_fld & 0x1f),
            (m_int_static_fld << 5) + (Q & 0x1f)
        );
    }

    static void f() {
        if(abool())
            xxxxb(ad - 2, J, G, ad - 1);
        else
            xxxxb(G, J, ad - 2, ad - 1);
    }

    static void l() {
        if(P != 0) {
            if(bbool())
                avoid8int(ac, N, s_int_static_fld, v_int_static_fld, O, O + 1, g_int_static_fld, g_int_static_fld + 1);
            else
                avoid8int(O, O + 1, g_int_static_fld, g_int_static_fld + 1, ac, N, s_int_static_fld, v_int_static_fld);
        } else
        if(bbool()) {
            bvoid2int(0, ac);
            bvoid2int(1, N);
            bvoid2int(2, s_int_static_fld);
            bvoid2int(3, v_int_static_fld);
            bvoid2int(4, O + 0);
            bvoid2int(5, O + 1);
            bvoid2int(6, g_int_static_fld + 0);
            bvoid2int(7, g_int_static_fld + 1);
        } else {
            bvoid2int(0, O + 0);
            bvoid2int(1, O + 1);
            bvoid2int(2, g_int_static_fld + 0);
            bvoid2int(3, g_int_static_fld + 1);
            bvoid2int(4, ac);
            bvoid2int(5, N);
            bvoid2int(6, s_int_static_fld);
            bvoid2int(7, v_int_static_fld);
        }
    }

    static byte *abyte0;
    static byte byte4,byte5,byte6,byte7,byte8,byte9,byte10,byte11,byte12,byte13,byte14;
    static int k7;
    static int i8,i9,i10,i11,i12,i13,i14,i15,i16,i17,i18,i19,i20,i21,i22,i23,i24,i25,i26,i27,i28,i29,i30,i31,i32,i33,i34,i35,i36,i37;
    static int j8,j9,j10,j11,j12,j13,j14,j15,j16,j17,j18,j19,j20,j21,j22,j23,j24,j25,j26,j27,j28,j29,j30,j31,j32,j33,j34,j35,j36,j37;
    static int k8,k9,k10,k11,k12,k13,k14,k15,k16,k17,k18,k19,k20,k21,k22,k23,k24,k25,k26,k27,k28,k29,k30,k31,k32,k33,k34,k35,k36;
    static int l10,l11,l12,l13,l14,l15,l16,l17,l18,l19,l20,l21,l22,l23,l24,l25,l26,l27,l28,l29,l30,l31,l32,l33,l34,l35,l36;
    static int i6,j6,i7,j7;
    static int i1, j1, k1, l1, i2, byte0, j2, k2, l2, i3, j3, k3, l3, i4, j4;
    static int ai[100];
    static int ai1[100];
    static int *ai2;
    static int k4;
    static int l4;
    static int i5;
    static int k5;
    static boolean flag;
    static int l5;
    static long l6;
    static boolean flag1;
    static boolean flag2;
    static boolean flag3;
    static boolean flag4;
    static boolean flag5;

    void init_values(void) {
        running=1;
        abyte0 = vram;

        i1 = 0;
        j1 = 0;
        k1 = 0;
        l1 = 0;
        i2 = 0;
        byte0 = 4;
        j2 = 255;
        k2 = 0;
        l2 = 1;
        i3 = 0;
        j3 = 0;
        k3 = 0;
        l3 = 0;
        i4 = 0;
        j4 = -1;
        ai2 = b_int_array1d_static_fld;
        k4 = (abyte0[ai2[7] + 65532] & 0xff) | (abyte0[ai2[7] + 65533] & 0xff) << 8;

        i5 = k4 + (l4 = ai2[k4 >> 13]);

        for(int j5 = 0; j5 < 100; j5++)
            ai1[j5] = 0;

        k5 = -50;
        flag = true;
        l5 = 0;
        l6 = SystemcurrentTimeMillis();
        c_long_static_fld = b_long_static_fld = 0L;
        flag1 = false;
        flag2 = i_boolean_static_fld;
        flag3 = false;
        flag4 = false;

        m();
        k_int_static_fld = 0;
        l_boolean_static_fld = j_boolean_static_fld = false;
        s_boolean_static_fld = true;
    }

//    #pragma optimize=no_code_motion
    void run_step() {

//new File().debugWrite("0:/vnes","\n"+Integer.toString(i5)+" = "+Integer.toString(k4));        
      //debug(ai2[7],abyte0[ai2[7] + 65532]);////////////
      //debug(ai2[7],abyte0[ai2[7] + 65533]);
        //if(true) return; //

   
   //new File().debugWrite("0:/vnes","\n"+Integer.toString(i5)+" = "+Integer.toString(k4));

//        do {
//            do {
                //main_loop_step();
 //debug("dodo",1,1);////////////            
                flag5 = k5 % A == 0;
                if(t_boolean_static_fld) {
                    l5 = p_int_static_fld + 1;
                    if(l5 >= 240)
                        l5 = -1;
                } else
                if((R & 0x18) != 0)
                    D = y;
                for(int k6 = 0; k6 <= 261; k6++) {
//debug("k6",k6,k6);////////////                                  
                    if(k6 == 261)
                        o_int_static_fld &= 0x3f;
                    else
                    if(k6 == 241)
                        o_int_static_fld |= 0x80;
                    else
                    if(k6 == 242) {
                        if((S & 0x80) != 0) {
                            k4 = i5 - l4;
                            abyte0[j2-- + 256] = (byte)(k4 >> 8);
                            abyte0[j2-- + 256] = (byte)k4;
                            abyte0[j2-- + 256] = (byte)(k2 | (k1 == 0 ? 2 : 0) | (k1 >> 7) << 7 | l3 << 6 | l2 << 2);
                            k4 = (abyte0[ai2[7] + 65530] & 0xff) | (abyte0[ai2[7] + 65531] & 0xff) << 8;
                            i5 = k4 + (l4 = ai2[k4 >> 13]);
                            i1 += 7;
                        }
                    } else
                    if(t_boolean_static_fld && k6 == l5 + 1 && (R & 0x18) != 0)
                        o_int_static_fld |= 0x40;
                    if(k4 != j4) {
                        if(t_boolean_static_fld) {
                            if(k6 <= 239) {
                                if(k6 > l5) {
                                    i1 += 116 * (240 - k6);
                                    k6 = 240;
                                } else
                                if(k6 < l5) {
                                    i1 += 116 * (l5 - k6);
                                    k6 = l5;
                                }
                            } else
                            if(k6 >= 242 && k6 <= 259) {
                                i1 += 116 * (260 - k6);
                                k6 = 260;
                            }
                        }
                        i1 += 116;
//debug(999,i5);                        
label0:
  if(!running) return;/////add
                        do {
                            if(i1 <= 0)
                                break;
///////debug("",i5,i1);  /used!!
//if(i1==0x70){
//  ;
//}
							//debug(i5,i5);                                
 //new File().debugWrite("0:/56","\n"+Integer.toString(i5)+" = "+Integer.toString(abyte0[i5]));
                            
//                            boolean flag6;
//                            boolean flag31;
//                            boolean flag32;
//                            boolean flag33;
//                            boolean flag34;
//                            boolean flag35;
//                            boolean flag36;
//                            boolean flag37;
//                            boolean flag38;
//                            boolean flag39;
//                            boolean flag40;
//                            boolean flag41;
//                            boolean flag42;
//                            boolean flag43;
//                            boolean flag44;
//                            boolean flag45;
//                            boolean flag46;
//                            boolean flag47;
//                            boolean flag48;
//                            boolean flag49;
//                            boolean flag50;
//                            boolean flag51;
//                            boolean flag52;
//                            boolean flag53;
//                            boolean flag54;
//                            boolean flag55;
//                            boolean flag56;
//                            boolean flag57;
//                            boolean flag58;
//                            boolean flag59;
//                            boolean flag60;
//                            boolean flag61;
//                            boolean flag62;
//                            boolean flag63;
//                            boolean flag64;
//                            boolean flag65;
//                            boolean flag66;
//                            boolean flag67;
//                            boolean flag68;
//                            boolean flag69;
//                            boolean flag70;
//                            boolean flag71;
//                            boolean flag72;
//                            boolean flag73;
//                            boolean flag74;
//                            boolean flag75;
//                            boolean flag76;
//                            boolean flag77;
//                            boolean flag78;
                            
               //31656
//                 31659//////////////
 /*if(i5==31659 || i5==31656){
   if(i5==31656) nnn=1;
   if(i5==31659 && nnn==1)
     printf("%d ",i5);                     
 }else nnn=0;
   */    //debug("i5",abyte0[i5],i5);////////////
/*  if(i5==16634){
     printf("%d=%d",i5, ++nnn);
     if(nnn==740)
       nnn=0;
  }*/
                            switch(abyte0[i5++]) {
                            default:
			                break;
			                           
                            case 0: // '\0'
                                k4 = (i5 - l4) + 1;
                                abyte0[j2-- + 256] = (byte)(k4 >> 8);
                                abyte0[j2-- + 256] = (byte)k4;
                                abyte0[j2-- + 256] = (byte)(k2 | (k1 == 0 ? 2 : 0) | (k1 >> 7) << 7 | l3 << 6 | l2 << 2);
                                k4 = (abyte0[ai2[7] + 65534] & 0xff) | (abyte0[ai2[7] + 65535] & 0xff) << 8;
                                i5 = k4 + (l4 = ai2[k4 >> 13]);
                                j3 = 1;
                                l2 = 1;
                                i1 -= 7;
                                continue;

                            case 1: // '\001'
                                k7 = (abyte0[i5++] + l1) & 0xff;
                                j1 |= (aint1int(abyte0[k7] & 0xff) | (abyte0[k7 + 1] & 0xff) << 8);
                                k1 = j1;
                                i1 -= 6;
                                continue;

                            case 2: // '\002'
                                //boolean flag7 = true;
                                continue;

                            case 3: // '\003'
                                //boolean flag8 = true;
                                continue;

                            case 4: // '\004'
                                i5++;
                                i1 -= 3;
                                continue;

                            case 5: // '\005'
                                j1 |= abyte0[abyte0[i5++] & 0xff] & 0xff;
                                k1 = j1;
                                i1 -= 3;
                                continue;

                            case 6: // '\006'
                                i8 = abyte0[i5++] & 0xff;
                                j8 = abyte0[i8] & 0xff;
                                k2 = j8 >> 7;
                                abyte0[i8] = (byte)(k1 = j8 << 1 & 0xff);
                                i1 -= 5;
                                continue;

                            case 7: // '\007'
                                //boolean flag9 = true;
                                continue;

                            case 8: // '\b'
                                abyte0[j2-- + 256] = (byte)(k2 | (k1 == 0 ? 2 : 0) | (k1 >> 7) << 7 | l3 << 6 | l2 << 2);
                                i1 -= 3;
                                continue;

                            case 9: // '\t'
                                j1 |= abyte0[i5++] & 0xff;
                                k1 = j1;
                                i1 -= 2;
                                continue;

                            case 10: // '\n'
                                k2 = j1 >> 7;
                                k1 = j1 = j1 << 1 & 0xff;
                                i1 -= 2;
                                continue;

                            case 11: // '\013'
                                //boolean flag10 = true;
                                continue;

                            case 12: // '\f'
                                i5 += 2;
                                i1 -= 4;
                                continue;

                            case 13: // '\r'
                                nn=abyte0[i5++] & 0xff;
                                j1 |= aint1int(nn | (abyte0[i5++] & 0xff) << 8);
                                k1 = j1;
                                i1 -= 4;
                                continue;

                            case 14: // '\016'
                                nn=abyte0[i5++] & 0xff;
                                k8 = nn | (abyte0[i5++] & 0xff) << 8;
                                i9 = aint1int(k8);
                                k2 = i9 >> 7;
                                avoid2int(k8, k1 = i9 << 1 & 0xff);
                                i1 -= 6;
                                continue;

                            case 15: // '\017'
                                //boolean flag11 = true;
                                continue;

                            case 16: // '\020'
                            //debug("16-1",k1,i5);
                                if(k1 >> 7 == 0) {
                                    byte byte2 = abyte0[i5++];
                                    i5 += byte2;
                                    i1 -= 3;
                                    if(byte2 < 0 && byte2 >= -5) {
                                        byte byte3 = abyte0[i5];
                                        if(byte2 == -5 && (byte3 == -83 || byte3 == 44))
                                            i1 = 0;
                                        else
                                        if(byte2 == -4 && byte3 == 6)
                                            i1 = 0;
                                    }
                                } else {
                                    i5++;
                                    i1 -= 2;
                                }
                              //  debug("16-2",k1,i5);
                                continue;

                            case 17: // '\021'
                                j9 = (abyte0[i5++]) & 0xff;
                                j1 |= (aint1int((abyte0[j9] & 0xff) | (abyte0[j9 + 1] & 0xff) << 8) + i2);
                                k1 = j1;
                                i1 -= 5;
                                continue;

                            case 18: // '\022'
                                //boolean flag12 = true;
                                continue;

                            case 19: // '\023'
                                //boolean flag13 = true;
                                continue;

                            case 20: // '\024'
                                i5++;
                                i1 -= 4;
                                continue;

                            case 21: // '\025'
                                j1 |= (abyte0[(abyte0[i5++] + l1) & 0xff]) & 0xff;
                                k1 = j1;
                                i1 -= 4;
                                continue;

                            case 22: // '\026'
                                k9 = (abyte0[i5++] + l1) & 0xff;
                                i10 = abyte0[k9] & 0xff;
                                k2 = i10 >> 7;
                                abyte0[k9] = (byte)(k1 = i10 << 1 & 0xff);
                                i1 -= 6;
                                continue;

                            case 23: // '\027'
                                //boolean flag14 = true;
                                continue;

                            case 24: // '\030'
                                k2 = 0;
                                i1 -= 2;
                                continue;

                            case 25: // '\031'
                                nn=abyte0[i5++] & 0xff;
                                j1 |= aint1int((nn | (abyte0[i5++] & 0xff) << 8) + i2);
                                k1 = j1;
                                i1 -= 4;
                                continue;

                            case 26: // '\032'
                                i1 -= 2;
                                continue;

                            case 27: // '\033'
                                //boolean flag15 = true;
                                continue;

                            case 28: // '\034'
                                i5 += 2;
                                i1 -= 4;
                                continue;

                            case 29: // '\035'
                                nn=abyte0[i5++] & 0xff;
                                j1 |= aint1int((nn | (abyte0[i5++] & 0xff) << 8) + l1);
                                k1 = j1;
                                i1 -= 5;
                                continue;

                            case 30: // '\036'
                                nn=abyte0[i5++] & 0xff;
                                j10 = (nn | (abyte0[i5++] & 0xff) << 8) + l1;
                                k10 = aint1int(j10);
                                k2 = k10 >> 7;
                                avoid2int(j10, k1 = k10 << 1 & 0xff);
                                i1 -= 7;
                                continue;

                            case 31: // '\037'
                                //boolean flag16 = true;
                                continue;

                            case 32: // ' '
                                k4 = (i5 - l4) + 1;
                                abyte0[j2-- + 256] = (byte)(k4 >> 8);
                                abyte0[j2-- + 256] = (byte)k4;
                                k4 = (abyte0[i5] & 0xff) | (abyte0[i5 + 1] & 0xff) << 8;
                                i5 = k4 + (l4 = ai2[k4 >> 13]);
                                i1 -= 6;
                                continue;

                            case 33: // '!'
                                l10 = (abyte0[i5++] + l1) & 0xff;
                                j1 &= aint1int((abyte0[l10] & 0xff) | (abyte0[l10 + 1] & 0xff) << 8);
                                k1 = j1;
                                i1 -= 6;
                                continue;

                            case 34: // '"'
                                //boolean flag17 = true;
                                continue;

                            case 35: // '#'
                                //boolean flag18 = true;
                                continue;

                            case 36: // '$'
                                i11 = abyte0[abyte0[i5++] & 0xff] & 0xff;
                                l3 = i11 >> 6 & 1;
                                byte4 = abyte0[i5];
                                if(byte4 == 16 || byte4 == 48)
                                    k1 = i11;
                                else
                                    k1 = j1 & i11;
                                i1 -= 3;
                                continue;

                            case 37: // '%'
                                j1 &= abyte0[abyte0[i5++] & 0xff] & 0xff;
                                k1 = j1;
                                i1 -= 3;
                                continue;

                            case 38: // '&'
                                j11 = abyte0[i5++] & 0xff;
                                k11 = abyte0[j11] & 0xff;
                                k11 = k11 << 1 | k2;
                                k2 = k11 >> 8;
                                abyte0[j11] = (byte)(k1 = k11 & 0xff);
                                i1 -= 5;
                                continue;

                            case 39: // '\''
                                //boolean flag19 = true;
                                continue;

                            case 40: // '('
                                l11 = abyte0[++j2 + 256] & 0xff;
                                k2 = l11 & 1;
                                k1 = ((l11 >> 1 & 1) == 0 ? 1 : 0) | (l11 >> 7) << 7;
                                l3 = l11 >> 6 & 1;
                                l2 = l11 >> 2 & 1;
                                i1 -= 4;
                                continue;

                            case 41: // ')'
                                j1 &= abyte0[i5++] & 0xff;
                                k1 = j1;
                                i1 -= 2;
                                continue;

                            case 42: // '*'
                                j1 = j1 << 1 | k2;
                                k2 = j1 >> 8;
                                j1 &= 0xff;
                                k1 = j1;
                                i1 -= 2;
                                continue;

                            case 43: // '+'
                                //boolean flag20 = true;
                                continue;

                            case 44: // ','
                                nn=abyte0[i5++] & 0xff;
                                i12 = aint1int( nn| (abyte0[i5++] & 0xff) << 8);
                                l3 = i12 >> 6 & 1;
                                byte5 = abyte0[i5];
                                if(byte5 == 16 || byte5 == 48)
                                    k1 = i12;
                                else
                                    k1 = j1 & i12;
                                i1 -= 4;
                                continue;

                            case 45: // '-'
                                nn=abyte0[i5++] & 0xff;
                                j1 &= aint1int(nn | (abyte0[i5++] & 0xff) << 8);
                                k1 = j1;
                                i1 -= 4;
                                continue;

                            case 46: // '.'
                                nn=abyte0[i5++] & 0xff;
                                j12 = nn | (abyte0[i5++] & 0xff) << 8;
                                k12 = aint1int(j12);
                                k12 = k12 << 1 | k2;
                                k2 = k12 >> 8;
                                avoid2int(j12, k1 = k12 & 0xff);
                                i1 -= 6;
                                continue;

                            case 47: // '/'
                                //boolean flag21 = true;
                                continue;

                            case 48: // '0'
                                if(k1 >> 7 != 0) {
                                    byte6 = abyte0[i5++];
                                    i5 += byte6;
                                    i1--;
                                } else {
                                    i5++;
                                }
                                i1 -= 2;
                                continue;

                            case 49: // '1'
                                l12 = abyte0[i5++] & 0xff;
                                j1 &= aint1int(((abyte0[l12] & 0xff) | (abyte0[l12 + 1] & 0xff) << 8) + i2);
                                k1 = j1;
                                i1 -= 5;
                                continue;

                            case 50: // '2'
                                //boolean flag22 = true;
                                continue;

                            case 51: // '3'
                                //boolean flag23 = true;
                                continue;

                            case 52: // '4'
                                i5++;
                                i1 -= 4;
                                continue;

                            case 53: // '5'
                                j1 &= abyte0[(abyte0[i5++] + l1) & 0xff] & 0xff;
                                k1 = j1;
                                i1 -= 4;
                                continue;

                            case 54: // '6'
                                i13 = (abyte0[i5++] + l1) & 0xff;
                                j13 = abyte0[i13] & 0xff;
                                j13 = j13 << 1 | k2;
                                k2 = j13 >> 8;
                                abyte0[i13] = (byte)(k1 = j13 & 0xff);
                                i1 -= 6;
                                continue;

                            case 55: // '7'
                                //boolean flag24 = true;
                                continue;

                            case 56: // '8'
                                k2 = 1;
                                i1 -= 2;
                                continue;

                            case 57: // '9'
                                nn=abyte0[i5++] & 0xff;
                                j1 &= aint1int((nn | (abyte0[i5++] & 0xff) << 8) + i2);
                                k1 = j1;
                                i1 -= 4;
                                continue;

                            case 58: // ':'
                                i1 -= 2;
                                continue;

                            case 59: // ';'
                                //boolean flag25 = true;
                                continue;

                            case 60: // '<'
                                i5 += 2;
                                i1 -= 4;
                                continue;

                            case 61: // '='
                                nn=abyte0[i5++] & 0xff;
                                j1 &= aint1int((nn | (abyte0[i5++] & 0xff) << 8) + l1);
                                k1 = j1;
                                i1 -= 5;
                                continue;

                            case 62: // '>'
                                nn=abyte0[i5++] & 0xff;
                                k13 = (nn | (abyte0[i5++] & 0xff) << 8) + l1;
                                l13 = aint1int(k13);
                                l13 = l13 << 1 | k2;
                                k2 = l13 >> 8;
                                avoid2int(k13, k1 = l13 & 0xff);
                                i1 -= 7;
                                continue;

                            case 63: // '?'
                                //boolean flag26 = true;
                                continue;

                            case 64: // '@'
                                i14 = abyte0[++j2 + 256] & 0xff;
                                k2 = i14 & 1;
                                k1 = ((i14 >> 1 & 1) == 0 ? 1 : 0) | (i14 >> 7) << 7;
                                l3 = i14 >> 6 & 1;
                                l2 = i14 >> 2 & 1;
                                nn=abyte0[++j2 + 256] & 0xff;
                                k4 = nn | (abyte0[++j2 + 256] & 0xff) << 8;
                                i5 = k4 + (l4 = ai2[k4 >> 13]);
                                i1 -= 6;
                                continue;

                            case 65: // 'A'
                                j14 = (abyte0[i5++] + l1) & 0xff;
                                j14 = (abyte0[j14] & 0xff) | (abyte0[j14 + 1] & 0xff) << 8;
                                j1 ^= aint1int(j14);
                                k1 = j1;
                                i1 -= 6;
                                continue;

                            case 66: // 'B'
                                //boolean flag27 = true;
                                continue;

                            case 67: // 'C'
                                //boolean flag28 = true;
                                continue;

                            case 68: // 'D'
                                i5++;
                                i1 -= 3;
                                continue;

                            case 69: // 'E'
                                j1 ^= abyte0[abyte0[i5++] & 0xff] & 0xff;
                                k1 = j1;
                                i1 -= 3;
                                continue;

                            case 70: // 'F'
                                k14 = abyte0[i5++] & 0xff;
                                l14 = abyte0[k14] & 0xff;
                                k2 = l14 & 1;
                                abyte0[k14] = (byte)(k1 = l14 >> 1);
                                i1 -= 5;
                                continue;

                            case 71: // 'G'
                                //boolean flag29 = true;
                                continue;

                            case 72: // 'H'
                                abyte0[j2-- + 256] = (byte)j1;
                                i1 -= 3;
                                continue;

                            case 73: // 'I'
                                j1 ^= abyte0[i5++] & 0xff;
                                k1 = j1;
                                i1 -= 2;
                                continue;

                            case 74: // 'J'
                                k2 = j1 & 1;
                                j1 >>= 1;
                                k1 = j1;
                                i1 -= 2;
                                continue;

                            case 75: // 'K'
                                //boolean flag30 = true;
                                continue;

                            case 76: // 'L'
                                i15 = i5 - 1;
                                k4 = (abyte0[i5] & 0xff) | (abyte0[i5 + 1] & 0xff) << 8;
                                i5 = k4 + (l4 = ai2[k4 >> 13]);
                                i1 -= 3;
                                if(i5 == j4 || i5 == i15) {
                                    i1 = 0;
                                    continue;
                                }
                                if(!flag)
                                    continue;
                                j15 = 1;
                                k15 = 0;
                                do {
                                    if(k15 >= 100)
                                        break;
                                    if(ai[k15] == i5) {
                                        ai1[k15]++;
                                        j15 = 0;
                                        break;
                                    }
                                    k15++;
                                } while(true);
                                if(j15 <= 0)
                                    continue;
                                k15 = 0;
                                do {
                                    if(k15 >= 100)
                                      goto label0;   //  continue
                                    if(ai1[k15] == 0) {
                                        ai1[k15]++;
                                        ai[k15] = i5;
                                        goto label0;  //continue
                                    }
                                    k15++;
                                } while(true);

                            case 77: // 'M'
                                nn=abyte0[i5++] & 0xff;
                                j1 ^= aint1int(nn | (abyte0[i5++] & 0xff) << 8);
                                k1 = j1;
                                i1 -= 4;
                                break;

                            case 78: // 'N'
                                nn=abyte0[i5++] & 0xff;
                                l15 = nn | (abyte0[i5++] & 0xff) << 8;
                                i16 = aint1int(l15);
                                k2 = i16 & 1;
                                avoid2int(l15, k1 = i16 >> 1);
                                i1 -= 6;
                                break;

                            case 79: // 'O'
//                                flag31 = true;
                                break;

                            case 80: // 'P'
                                if(l3 == 0) {
                                    byte7 = abyte0[i5++];
                                    i5 += byte7;
                                    i1 -= 3;
                                    if(byte7 < 0 && byte7 >= -5) {
                                        byte8 = abyte0[i5];
                                        if(byte7 == -5 && byte8 == 44)
                                            i1 = 0;
                                    }
                                } else {
                                    i5++;
                                    i1 -= 2;
                                }
                                break;

                            case 81: // 'Q'
                                j16 = abyte0[i5++] & 0xff;
                                j1 ^= aint1int(((abyte0[j16] & 0xff) | (abyte0[j16 + 1] & 0xff) << 8) + i2);
                                k1 = j1;
                                i1 -= 5;
                                break;

                            case 82: // 'R'
//                                flag32 = true;
                                break;

                            case 83: // 'S'
//                                flag33 = true;
                                break;

                            case 84: // 'T'
                                i5++;
                                i1 -= 4;
                                break;

                            case 85: // 'U'
                                j1 ^= abyte0[(abyte0[i5++] + l1) & 0xff] & 0xff;
                                k1 = j1;
                                i1 -= 4;
                                break;

                            case 86: // 'V'
                                k16 = (abyte0[i5++] + l1) & 0xff;
                                l16 = abyte0[k16] & 0xff;
                                k2 = l16 & 1;
                                abyte0[k16] = (byte)(k1 = l16 >> 1);
                                i1 -= 6;
                                break;

                            case 87: // 'W'
//                                flag34 = true;
                                break;

                            case 88: // 'X'
                                l2 = 0;
                                i1 -= 2;
                                break;

                            case 89: // 'Y'
                                nn=abyte0[i5++] & 0xff;
                                j1 ^= aint1int((nn | (abyte0[i5++] & 0xff) << 8) + i2);
                                k1 = j1;
                                i1 -= 4;
                                break;

                            case 90: // 'Z'
                                i1 -= 2;
                                break;

                            case 91: // '['
//                                flag35 = true;
                                break;

                            case 92: // '\\'
                                i5 += 2;
                                i1 -= 4;
                                break;

                            case 93: // ']'
                                nn=abyte0[i5++] & 0xff;
                                j1 ^= aint1int((nn | (abyte0[i5++] & 0xff) << 8) + l1);
                                k1 = j1;
                                i1 -= 5;
                                break;

                            case 94: // '^'
                                nn=abyte0[i5++] & 0xff;
                                i17 = (nn | (abyte0[i5++] & 0xff) << 8) + l1;
                                j17 = aint1int(i17);
                                k2 = j17 & 1;
                                avoid2int(i17, k1 = j17 >> 1);
                                i1 -= 7;
                                break;

                            case 95: // '_'
//                                flag36 = true;
                                break;

                            case 96: // '`'
                              nn=abyte0[++j2 + 256] & 0xff;
                                k4 = (nn | (abyte0[++j2 + 256] & 0xff) << 8) + 1;
                                i5 = k4 + (l4 = ai2[k4 >> 13]);
                                i1 -= 4;
                                break;

                            case 97: // 'a'
                                k17 = (abyte0[i5++] + l1) & 0xff;
                                l17 = aint1int((abyte0[k17] & 0xff) | (abyte0[k17 + 1] & 0xff) << 8);
                                i18 = j1 + l17 + k2;
                                l3 = (~(j1 ^ l17) & (j1 ^ i18)) >> 7 & 1;
                                k2 = i18 >> 8;
                                j1 = i18 & 0xff;
                                k1 = j1;
                                i1 -= 6;
                                break;

                            case 98: // 'b'
//                                flag37 = true;
                                break;

                            case 99: // 'c'
//                                flag38 = true;
                                break;

                            case 100: // 'd'
                                i5++;
                                i1 -= 3;
                                break;

                            case 101: // 'e'
                                j18 = abyte0[abyte0[i5++] & 0xff] & 0xff;
                                k18 = j1 + j18 + k2;
                                l3 = (~(j1 ^ j18) & (j1 ^ k18)) >> 7 & 1;
                                k2 = k18 >> 8;
                                k1 = j1 = k18 & 0xff;
                                i1 -= 3;
                                break;

                            case 102: // 'f'
                                l18 = abyte0[i5++] & 0xff;
                                i19 = abyte0[l18] & 0xff;
                                j19 = k2 << 7;
                                k2 = i19 & 1;
                                abyte0[l18] = (byte)(k1 = i19 >> 1 | j19);
                                i1 -= 5;
                                break;

                            case 103: // 'g'
//                                flag39 = true;
                                break;

                            case 104: // 'h'
                                k1 = j1 = abyte0[++j2 + 256] & 0xff;
                                i1 -= 4;
                                break;

                            case 105: // 'i'
                                k19 = abyte0[i5++] & 0xff;
                                l19 = j1 + k19 + k2;
                                l3 = (~(j1 ^ k19) & (j1 ^ l19)) >> 7 & 1;
                                k2 = l19 >> 8;
                                k1 = j1 = l19 & 0xff;
                                i1 -= 2;
                                break;

                            case 106: // 'j'
                                i20 = k2;
                                k2 = j1 & 1;
                                k1 = j1 = j1 >> 1 | i20 << 7;
                                i1 -= 2;
                                break;

                            case 107: // 'k'
//                                flag40 = true;
                                break;

                            case 108: // 'l'
                                nn=abyte0[i5++] & 0xff;
                                j20 = nn | (abyte0[i5++] & 0xff) << 8;
                                if((j20 & 0xff) == 255)
                                    k4 = aint1int(j20) | aint1int(j20 & 0xff00) << 8;
                                else
                                    k4 = aint1int(j20) | aint1int(j20 + 1) << 8;
                                i5 = k4 + (l4 = ai2[k4 >> 13]);
                                i1 -= 5;
                                break;

                            case 109: // 'm'
                                nn=abyte0[i5++] & 0xff;
                                k20 = aint1int(nn | (abyte0[i5++] & 0xff) << 8);
                                l20 = j1 + k20 + k2;
                                l3 = (~(j1 ^ k20) & (j1 ^ l20)) >> 7 & 1;
                                k2 = l20 >> 8;
                                k1 = j1 = l20 & 0xff;
                                i1 -= 4;
                                break;

                            case 110: // 'n'
                                nn=abyte0[i5++] & 0xff;
                                i21 = nn | (abyte0[i5++] & 0xff) << 8;
                                j21 = aint1int(i21);
                                k21 = k2 << 7;
                                k2 = j21 & 1;
                                avoid2int(i21, k1 = j21 >> 1 | k21);
                                i1 -= 6;
                                break;

                            case 111: // 'o'
//                                flag41 = true;
                                break;

                            case 112: // 'p'
                                if(l3 != 0) {
                                    byte9 = abyte0[i5++];
                                    i5 += byte9;
                                    i1--;
                                } else {
                                    i5++;
                                }
                                i1 -= 4;
                                break;

                            case 113: // 'q'
                                l21 = abyte0[i5++] & 0xff;
                                i22 = aint1int(((abyte0[l21] & 0xff) | (abyte0[l21 + 1] & 0xff) << 8) + i2);
                                j22 = j1 + i22 + k2;
                                l3 = (~(j1 ^ i22) & (j1 ^ j22)) >> 7 & 1;
                                k2 = j22 >> 8;
                                k1 = j1 = j22 & 0xff;
                                i1 -= 5;
                                break;

                            case 114: // 'r'
//                                flag42 = true;
                                break;

                            case 115: // 's'
//                                flag43 = true;
                                break;

                            case 116: // 't'
                                i5++;
                                i1 -= 4;
                                break;

                            case 117: // 'u'
                                k22 = abyte0[(abyte0[i5++] + l1) & 0xff] & 0xff;
                                l22 = j1 + k22 + k2;
                                l3 = (~(j1 ^ k22) & (j1 ^ l22)) >> 7 & 1;
                                k2 = l22 >> 8;
                                k1 = j1 = l22 & 0xff;
                                i1 -= 4;
                                break;

                            case 118: // 'v'
                                i23 = (abyte0[i5++] + l1) & 0xff;
                                j23 = abyte0[i23] & 0xff;
                                k23 = k2 << 7;
                                k2 = j23 & 1;
                                abyte0[i23] = (byte)(k1 = j23 >> 1 | k23);
                                i1 -= 6;
                                break;

                            case 119: // 'w'
//                                flag44 = true;
                                break;

                            case 120: // 'x'
                                l2 = 1;
                                i1 -= 2;
                                break;

                            case 121: // 'y'
                                nn=abyte0[i5++] & 0xff;
                                l23 = aint1int((nn | (abyte0[i5++] & 0xff) << 8) + i2);
                                i24 = j1 + l23 + k2;
                                l3 = (~(j1 ^ l23) & (j1 ^ i24)) >> 7 & 1;
                                k2 = i24 >> 8;
                                k1 = j1 = i24 & 0xff;
                                i1 -= 4;
                                break;

                            case 122: // 'z'
                                i1 -= 2;
                                break;

                            case 123: // '{'
//                                flag45 = true;
                                break;

                            case 124: // '|'
                                i5 += 2;
                                i1 -= 4;
                                break;

                            case 125: // '}'
                                nn=abyte0[i5++] & 0xff;
                                j24 = aint1int((nn | (abyte0[i5++] & 0xff) << 8) + l1);
                                k24 = j1 + j24 + k2;
                                l3 = (~(j1 ^ j24) & (j1 ^ k24)) >> 7 & 1;
                                k2 = k24 >> 8;
                                k1 = j1 = k24 & 0xff;
                                i1 -= 5;
                                break;

                            case 126: // '~'
                                nn=abyte0[i5++] & 0xff;
                                l24 = (nn | (abyte0[i5++] & 0xff) << 8) + l1;
                                i25 = aint1int(l24);
                                j25 = k2 << 7;
                                k2 = i25 & 1;
                                avoid2int(l24, k1 = i25 >> 1 | j25);
                                i1 -= 7;
                                break;

                            case 127: // '\177'
//                                flag46 = true;
                                break;

                            case -128: 
                                i5++;
                                i1 -= 2;
                                break;

                            case -127: 
                                k25 = (abyte0[i5++] + l1) & 0xff;
                                avoid2int((abyte0[k25] & 0xff) | (abyte0[k25 + 1] & 0xff) << 8, j1);
                                i1 -= 6;
                                break;

                            case -126: 
                                i5++;
                                i1 -= 2;
                                break;

                            case -125: 
//                                flag47 = true;
                                break;

                            case -124: 
                                abyte0[abyte0[i5++] & 0xff] = (byte)i2;
                                i1 -= 3;
                                break;

                            case -123: 
                                abyte0[abyte0[i5++] & 0xff] = (byte)j1;
                                i1 -= 3;
                                break;

                            case -122: 
                                abyte0[abyte0[i5++] & 0xff] = (byte)l1;
                                i1 -= 3;
                                break;

                            case -121: 
//                                flag48 = true;
                                break;

                            case -120: 
                                k1 = i2 = (i2 - 1) & 0xff;
                                i1 -= 2;
                                break;

                            case -119: 
                                i5++;
                                i1 -= 2;
                                break;

                            case -118: 
                                k1 = j1 = l1;
                                i1 -= 2;
                                break;

                            case -117: 
//                                flag49 = true;
                                break;

                            case -116: 
                                nn=abyte0[i5++] & 0xff;
                                avoid2int(nn | (abyte0[i5++] & 0xff) << 8, i2);
                                i1 -= 4;
                                break;

                            case -115: 
                                nn=abyte0[i5++] & 0xff;
                                avoid2int(nn | (abyte0[i5++] & 0xff) << 8, j1);
                                i1 -= 4;
                                break;

                            case -114: 
                                nn=abyte0[i5++] & 0xff;
                                avoid2int(nn | (abyte0[i5++] & 0xff) << 8, l1);
                                i1 -= 4;
                                break;

                            case -113: 
//                                flag50 = true;
                                break;

                            case -112: 
                                if(k2 == 0) {
                                    byte10 = abyte0[i5++];
                                    i5 += byte10;
                                    i1--;
                                } else {
                                    i5++;
                                }
                                i1 -= 2;
                                break;

                            case -111: 
                                l25 = abyte0[i5++] & 0xff;
                                avoid2int(((abyte0[l25] & 0xff) | (abyte0[l25 + 1] & 0xff) << 8) + i2, j1);
                                i1 -= 6;
                                break;

                            case -110: 
//                                flag51 = true;
                                break;

                            case -109: 
//                                flag52 = true;
                                break;

                            case -108: 
                                abyte0[(abyte0[i5++] + l1) & 0xff] = (byte)i2;
                                i1 -= 4;
                                break;

                            case -107: 
                                abyte0[(abyte0[i5++] + l1) & 0xff] = (byte)j1;
                                i1 -= 4;
                                break;

                            case -106: 
                                abyte0[(abyte0[i5++] + i2) & 0xff] = (byte)l1;
                                i1 -= 4;
                                break;

                            case -105: 
//                                flag53 = true;
                                break;

                            case -104: 
                                k1 = j1 = i2;
                                i1 -= 2;
                                break;

                            case -103: 
                                nn=abyte0[i5++] & 0xff;
                                avoid2int((nn | (abyte0[i5++] & 0xff) << 8) + i2, j1);
                                i1 -= 5;
                                break;

                            case -102: 
                                j2 = l1;
                                i1 -= 2;
                                break;

                            case -101: 
//                                flag54 = true;
                                break;

                            case -100: 
//                                flag55 = true;
                                break;

                            case -99: 
                                nn=abyte0[i5++] & 0xff;
                                avoid2int((nn | (abyte0[i5++] & 0xff) << 8) + l1, j1);
                                i1 -= 5;
                                break;

                            case -98: 
//                                flag56 = true;
                                break;

                            case -97: 
//                                flag57 = true;
                                break;

                            case -96: 
                                k1 = i2 = abyte0[i5++] & 0xff;
                                i1 -= 2;
                                break;

                            case -95: 
                                i26 = (abyte0[i5++] + l1) & 0xff;
                                k1 = j1 = aint1int((abyte0[i26] & 0xff) | (abyte0[i26 + 1] & 0xff) << 8);
                                i1 -= 6;
                                break;

                            case -94: 
                                k1 = l1 = abyte0[i5++] & 0xff;
                                i1 -= 2;
                                break;

                            case -93: 
//                                flag58 = true;
                                break;

                            case -92: 
                                k1 = i2 = abyte0[abyte0[i5++] & 0xff] & 0xff;
                                i1 -= 3;
                                break;

                            case -91: 
                                k1 = j1 = abyte0[abyte0[i5++] & 0xff] & 0xff;
                                i1 -= 3;
                                break;

                            case -90: 
                                k1 = l1 = abyte0[abyte0[i5++] & 0xff] & 0xff;
                                i1 -= 3;
                                break;

                            case -89: 
//                                flag59 = true;
                                break;

                            case -88: 
                                k1 = i2 = j1;
                                i1 -= 2;
                                break;

                            case -87: 
                                k1 = j1 = abyte0[i5++] & 0xff;
                                i1 -= 2;
                                break;

                            case -86: 
                                k1 = l1 = j1;
                                i1 -= 2;
                                break;

                            case -85: 
//                                flag60 = true;
                                break;

                            case -84: 
                                nn=abyte0[i5++] & 0xff;
                                k1 = i2 = aint1int(nn | (abyte0[i5++] & 0xff) << 8);
                                i1 -= 4;
                                break;

                            case -83: 
                               //debug("-83-1",abyte0[i5],abyte0[i5+1]);
                                nn=abyte0[i5++] & 0xff;
                                k1 = j1 = aint1int(nn | (abyte0[i5++] & 0xff) << 8);
                                i1 -= 4;
                                //debug("-83-2",k1,abyte0[i5-1]);                              
                                break;

                            case -82: 
                                nn=abyte0[i5++] & 0xff;
                                k1 = l1 = aint1int(nn | (abyte0[i5++] & 0xff) << 8);
                                i1 -= 4;
                                break;

                            case -81: 
//                                flag61 = true;
                                break;

                            case -80: 
                                if(k2 != 0) {
                                    byte11 = abyte0[i5++];
                                    i5 += byte11;
                                    i1 -= 3;
                                } else {
                                    i5++;
                                    i1 -= 2;
                                }
                                break;

                            case -79: 
                                j26 = abyte0[i5++] & 0xff;
                                k1 = j1 = aint1int(((abyte0[j26] & 0xff) | (abyte0[j26 + 1] & 0xff) << 8) + i2);
                                i1 -= 5;
                                break;

                            case -78: 
//                                flag62 = true;
                                break;

                            case -77: 
//                                flag63 = true;
                                break;

                            case -76: 
                                k1 = i2 = abyte0[(abyte0[i5++] + l1) & 0xff] & 0xff;
                                i1 -= 4;
                                break;

                            case -75: 
                                k1 = j1 = abyte0[(abyte0[i5++] + l1) & 0xff] & 0xff;
                                i1 -= 4;
                                break;

                            case -74: 
                                k1 = l1 = abyte0[(abyte0[i5++] + i2) & 0xff] & 0xff;
                                i1 -= 4;
                                break;

                            case -73: 
//                                flag64 = true;
                                break;

                            case -72: 
                                l3 = 0;
                                i1 -= 2;
                                break;

                            case -71: 
                                nn=abyte0[i5++] & 0xff;
                                k1 = j1 = aint1int((nn | (abyte0[i5++] & 0xff) << 8) + i2);
                                i1 -= 4;
                                break;

                            case -70: 
                                k1 = l1 = j2;
                                i1 -= 2;
                                break;

                            case -69: 
//                                flag65 = true;
                                break;

                            case -68: 
                              nn=abyte0[i5++] & 0xff;
                                k1 = i2 = aint1int((nn | (abyte0[i5++] & 0xff) << 8) + l1);
                                i1 -= 4;
                                break;

                            case -67: 
                                nn=abyte0[i5++] & 0xff;
                                k1 = j1 = aint1int((nn | (abyte0[i5++] & 0xff) << 8) + l1);
                                i1 -= 4;
                                break;

                            case -66: 
                              nn=abyte0[i5++] & 0xff;
                                k1 = l1 = aint1int((nn | (abyte0[i5++] & 0xff) << 8) + i2);
                                i1 -= 4;
                                break;

                            case -65: 
//                                flag66 = true;
                                break;

                            case -64: 
                                k26 = i2 - (abyte0[i5++] & 0xff);
                                k2 = k26 < 0 ? 0 : 1;
                                k1 = k26 & 0xff;
                                i1 -= 2;
                                break;

                            case -63: 
                                l26 = (abyte0[i5++] + l1) & 0xff;
                                l26 = j1 - aint1int((abyte0[l26] & 0xff) | (abyte0[l26 + 1] & 0xff) << 8);
                                k2 = l26 < 0 ? 0 : 1;
                                k1 = l26 & 0xff;
                                i1 -= 6;
                                break;

                            case -62: 
                                i5++;
                                i1 -= 2;
                                break;

                            case -61: 
//                                flag67 = true;
                                break;

                            case -60: 
                                i27 = i2 - (abyte0[abyte0[i5++] & 0xff] & 0xff);
                                k2 = i27 < 0 ? 0 : 1;
                                k1 = i27 & 0xff;
                                i1 -= 3;
                                break;

                            case -59: 
                                j27 = j1 - (abyte0[abyte0[i5++] & 0xff] & 0xff);
                                k2 = j27 < 0 ? 0 : 1;
                                k1 = j27 & 0xff;
                                i1 -= 3;
                                break;

                            case -58: 
                                k27 = abyte0[i5++] & 0xff;
                                k1 = --abyte0[k27] & 0xff;
                                i1 -= 5;
                                break;

                            case -57: 
//                                flag68 = true;
                                break;

                            case -56: 
                                k1 = i2 = (i2 + 1) & 0xff;
                                i1 -= 2;
                                break;

                            case -55: 
                                l27 = j1 - (abyte0[i5++] & 0xff);
                                k2 = l27 < 0 ? 0 : 1;
                                k1 = l27 & 0xff;
                                i1 -= 2;
                                break;

                            case -54: 
                                k1 = l1 = (l1 - 1) & 0xff;
                                i1 -= 2;
                                break;

                            case -53: 
//                                flag69 = true;
                                break;

                            case -52: 
                                nn=abyte0[i5++] & 0xff;
                                i28 = i2 - aint1int(nn | (abyte0[i5++] & 0xff) << 8);
                                k2 = i28 < 0 ? 0 : 1;
                                k1 = i28 & 0xff;
                                i1 -= 4;
                                break;

                            case -51: 
                               nn=abyte0[i5++] & 0xff;
                                j28 = j1 - aint1int(nn | (abyte0[i5++] & 0xff) << 8);
                                k2 = j28 < 0 ? 0 : 1;
                                k1 = j28 & 0xff;
                                i1 -= 4;
                                break;

                            case -50: 
                               nn=abyte0[i5++] & 0xff;
                                k28 = nn | (abyte0[i5++] & 0xff) << 8;
                                avoid2int(k28, k1 = (aint1int(k28) - 1) & 0xff);
                                i1 -= 6;
                                break;

                            case -49: 
//                                flag70 = true;
                                break;

                            case -48: 
                                if(k1 != 0) {
                                    byte12 = abyte0[i5++];
                                    i5 += byte12;
                                    i1 -= 3;
                                    if(byte12 < 0 && byte12 >= -5) {
                                        i29 = abyte0[i5];
                                        if(byte12 == -2)
                                            i1 = 0;
                                        else
                                        if(byte12 == -4 && i29 == -91)
                                            i1 = 0;
                                        else
                                        if(byte12 == -5 && i29 == -83)
                                            i1 = 0;
                                        else
                                        if(byte12 == -3 && i29 == -54) {
                                            i29 = i1 / 5;
                                            if(l1 < i29)
                                                i29 = l1;
                                            l1 -= i29;
                                            i1 -= i29 * 5;
                                            if(l1 == 0) {
                                                k1 = l1;
                                                i1++;
                                                i5 -= byte12;
                                            }
                                        } else
                                        if(byte12 == -4 && i29 == -22 && abyte0[i5 + 1] == -54) {
                                            i29 = i1 / 7;
                                            if(l1 < i29)
                                                i29 = l1;
                                            l1 -= i29;
                                            i1 -= i29 * 7;
                                            if(l1 == 0) {
                                                k1 = l1;
                                                i1++;
                                                i5 -= byte12;
                                            }
                                        } else
                                        if(byte12 == -3 && i29 == -120) {
                                            j29 = i1 / 5;
                                            if(i2 < j29)
                                                j29 = i2;
                                            i2 -= j29;
                                            i1 -= j29 * 5;
                                            if(i2 == 0) {
                                                k1 = i2;
                                                i1++;
                                                i5 -= byte12;
                                            }
                                        }
                                    }
                                } else {
                                    i5++;
                                    i1 -= 2;
                                }
                                break;

                            case -47: 
                                l28 = abyte0[i5++] & 0xff;
                                l28 = j1 - aint1int(((abyte0[l28] & 0xff) | (abyte0[l28 + 1] & 0xff) << 8) + i2);
                                k2 = l28 < 0 ? 0 : 1;
                                k1 = l28 & 0xff;
                                i1 -= 5;
                                break;

                            case -46: 
//                                flag71 = true;
                                break;

                            case -45: 
//                                flag72 = true;
                                break;

                            case -44: 
                                i5++;
                                i1 -= 4;
                                break;

                            case -43: 
                                k29 = j1 - (abyte0[(abyte0[i5++] + l1) & 0xff] & 0xff);
                                k2 = k29 < 0 ? 0 : 1;
                                k1 = k29 & 0xff;
                                i1 -= 4;
                                break;

                            case -42: 
                                l29 = (abyte0[i5++] + l1) & 0xff;
                                k1 = --abyte0[l29] & 0xff;
                                i1 -= 6;
                                break;

                            case -41: 
//                                flag73 = true;
                                break;

                            case -40: 
                                i3 = 0;
                                i1 -= 2;
                                break;

                            case -39: 
                                nn=abyte0[i5++] & 0xff;
                                i30 = j1 - aint1int((nn | (abyte0[i5++] & 0xff) << 8) + i2);
                                k2 = i30 < 0 ? 0 : 1;
                                k1 = i30 & 0xff;
                                i1 -= 4;
                                break;

                            case -38: 
                                i1 -= 2;
                                break;

                            case -37: 
//                                flag74 = true;
                                break;

                            case -36: 
                                i5 += 2;
                                i1 -= 4;
                                break;

                            case -35: 
                              nn=abyte0[i5++] & 0xff;
                                j30 = j1 - aint1int((nn | (abyte0[i5++] & 0xff) << 8) + l1);
                                k2 = j30 < 0 ? 0 : 1;
                                k1 = j30 & 0xff;
                                i1 -= 5;
                                break;

                            case -34: 
                              nn=abyte0[i5++] & 0xff;
                                k30 = (nn | (abyte0[i5++] & 0xff) << 8) + l1;
                                avoid2int(k30, k1 = (aint1int(k30) - 1) & 0xff);
                                i1 -= 7;
                                break;

                            case -33: 
//                                flag75 = true;
                                break;

                            case -32: 
                                l30 = l1 - (abyte0[i5++] & 0xff);
                                k2 = l30 < 0 ? 0 : 1;
                                k1 = l30 & 0xff;
                                i1 -= 2;
                                break;

                            case -31: 
                                i31 = (abyte0[i5++] + l1) & 0xff;
                                j31 = aint1int((abyte0[i31] & 0xff) | (abyte0[i31 + 1] & 0xff) << 8);
                                k31 = j1 - j31 - (1 - k2);
                                l3 = ((j1 ^ j31) & (j1 ^ k31)) >> 7;
                                k2 = k31 < 0 ? 0 : 1;
                                k1 = j1 = k31 & 0xff;
                                i1 -= 6;
                                break;

                            case -30: 
                                i5++;
                                i1 -= 2;
                                break;

                            case -29: 
//                                flag76 = true;
                                break;

                            case -28: 
                                l31 = l1 - (abyte0[abyte0[i5++] & 0xff] & 0xff);
                                k2 = l31 < 0 ? 0 : 1;
                                k1 = l31 & 0xff;
                                i1 -= 3;
                                break;

                            case -27: 
                                i32 = abyte0[abyte0[i5++] & 0xff] & 0xff;
                                j32 = j1 - i32 - (1 - k2);
                                l3 = ((j1 ^ i32) & (j1 ^ j32)) >> 7;
                                k2 = j32 < 0 ? 0 : 1;
                                k1 = j1 = j32 & 0xff;
                                i1 -= 3;
                                break;

                            case -26: 
                                k32 = abyte0[i5++] & 0xff;
                                k1 = ++abyte0[k32] & 0xff;
                                i1 -= 5;
                                break;

                            case -25: 
//                                flag77 = true;
                                break;

                            case -24: 
                                k1 = l1 = (l1 + 1) & 0xff;
                                i1 -= 2;
                                break;

                            case -23: 
                                l32 = abyte0[i5++] & 0xff;
                                i33 = j1 - l32 - (1 - k2);
                                l3 = ((j1 ^ l32) & (j1 ^ i33)) >> 7;
                                k2 = i33 < 0 ? 0 : 1;
                                k1 = j1 = i33 & 0xff;
                                i1 -= 2;
                                break;

                            case -22: 
                                i1 -= 2;
                                break;

                            case -21: 
                                j33 = abyte0[i5++] & 0xff;
                                k33 = j1 - j33 - (1 - k2);
                                l3 = ((j1 ^ j33) & (j1 ^ k33)) >> 7;
                                k2 = k33 < 0 ? 0 : 1;
                                k1 = j1 = k33 & 0xff;
                                i1 -= 2;
                                break;

                            case -20: 
                              nn=abyte0[i5++] & 0xff;
                                l33 = l1 - aint1int(nn | (abyte0[i5++] & 0xff) << 8);
                                k2 = l33 < 0 ? 0 : 1;
                                k1 = l33 & 0xff;
                                i1 -= 4;
                                break;

                            case -19: 
                              nn=abyte0[i5++] & 0xff;
                                i34 = aint1int(nn | (abyte0[i5++] & 0xff) << 8);
                                j34 = j1 - i34 - (1 - k2);
                                l3 = ((j1 ^ i34) & (j1 ^ j34)) >> 7;
                                k2 = j34 < 0 ? 0 : 1;
                                k1 = j1 = j34 & 0xff;
                                i1 -= 4;
                                break;

                            case -18: 
                              nn=abyte0[i5++] & 0xff;
                                k34 = nn | (abyte0[i5++] & 0xff) << 8;
                                avoid2int(k34, k1 = (aint1int(k34) + 1) & 0xff);
                                i1 -= 6;
                                break;

                            case -17: 
//                                flag78 = true;
                                break;

                            case -16: 
                                if(k1 == 0) {
                                    byte13 = abyte0[i5++];
                                    i5 += byte13;
                                    i1 -= 3;
                                    if(byte13 < 0 && byte13 >= -5) {
                                        byte14 = abyte0[i5];
                                        if(byte13 == -4 && (byte14 == -59 || byte14 == -91))
                                            i1 = 0;
                                    }
                                } else {
                                    i5++;
                                    i1 -= 2;
                                }
                                break;

                            case -15: 
                                l34 = abyte0[i5++] & 0xff;
                                i35 = aint1int(((abyte0[l34] & 0xff) | (abyte0[l34 + 1] & 0xff) << 8) + i2);
                                j35 = j1 - i35 - (1 - k2);
                                l3 = ((j1 ^ i35) & (j1 ^ j35)) >> 7;
                                k2 = j35 < 0 ? 0 : 1;
                                k1 = j1 = j35 & 0xff;
                                i1 -= 5;
                                break;

                            case -14: 
//                                flag6 = true;
                                break;

                            case -13: 
//                                flag6 = true;
                                break;

                            case -12: 
                                i5++;
                                i1 -= 4;
                                break;

                            case -11: 
                                k35 = abyte0[(abyte0[i5++] + l1) & 0xff] & 0xff;
                                l35 = j1 - k35 - (1 - k2);
                                l3 = ((j1 ^ k35) & (j1 ^ l35)) >> 7;
                                k2 = l35 < 0 ? 0 : 1;
                                k1 = j1 = l35 & 0xff;
                                i1 -= 4;
                                break;

                            case -10: 
                                i36 = (abyte0[i5++] + l1) & 0xff;
                                k1 = ++abyte0[i36] & 0xff;
                                i1 -= 6;
                                break;

                            case -9: 
//                                flag6 = true;
                                break;

                            case -8: 
                                i3 = 1;
                                i1 -= 2;
                                break;

                            case -7: 
                              nn=abyte0[i5++] & 0xff;
                                j36 = aint1int((nn | (abyte0[i5++] & 0xff) << 8) + i2);
                                k36 = j1 - j36 - (1 - k2);
                                l3 = ((j1 ^ j36) & (j1 ^ k36)) >> 7;
                                k2 = k36 < 0 ? 0 : 1;
                                k1 = j1 = k36 & 0xff;
                                i1 -= 4;
                                break;

                            case -6: 
                                i1 -= 2;
                                break;

                            case -5: 
//                                flag6 = true;
                                break;

                            case -4: 
                                i5 += 2;
                                i1 -= 4;
                                break;

                            case -3: 
                              nn=abyte0[i5++] & 0xff;
                                l36 = aint1int((nn | (abyte0[i5++] & 0xff) << 8) + l1);
                                i37 = j1 - l36 - (1 - k2);
                                l3 = ((j1 ^ l36) & (j1 ^ i37)) >> 7;
                                k2 = i37 < 0 ? 0 : 1;
                                k1 = j1 = i37 & 0xff;
                                i1 -= 5;
                                break;

                            case -2: 
                              nn=abyte0[i5++] & 0xff;
                                j37 = (nn | (abyte0[i5++] & 0xff) << 8) + l1;
                                avoid2int(j37, k1 = (aint1int(j37) + 1) & 0xff);
                                i1 -= 7;
                                break;

                            case -1: 
//                                flag6 = true;
                                break;
                            }
                        } while(true);
                    }
                    if(flag2 && bint1int(k6) != 0 && l2 == 0) {
                        k4 = i5 - l4;
                        abyte0[j2-- + 256] = (byte)(k4 >> 8);
                        abyte0[j2-- + 256] = (byte)k4;
                        abyte0[j2-- + 256] = (byte)(k2 | (k1 == 0 ? 2 : 0) | (k1 >> 7) << 7 | l3 << 6 | l2 << 2);
                        k4 = (abyte0[ai2[7] + 65534] & 0xff) | (abyte0[ai2[7] + 65535] & 0xff) << 8;
                        i5 = k4 + (l4 = ai2[k4 >> 13]);
                        l2 = 1;
                        i1 += 7;
                    }
                    if(flag5 && t_boolean_static_fld && k6 == 240 && (R & 0x18) != 0) {
                        long l8 = SystemcurrentTimeMillis();
                        c_long_static_fld += l8 - l6;
                        //c c1 = main;
                        a_long_static_fld = k5;//c.??
                   //     try {
                            c0();
                   //     }
                   //     catch(Exception exception2) { }
                        l6 = SystemcurrentTimeMillis();
                        b_long_static_fld += l6 - l8;
                    }
                    //////////////////////////////////
                    if(t_boolean_static_fld || k6 >= 240 || (R & 0x18) == 0)
                        continue;

                    if(flag5) {
                        D &= 0xfbe0;
                        D |= y & 0x41f;
                        long l9 = SystemcurrentTimeMillis();
                        c_long_static_fld += l9 - l6;
                        //c c2 = main;
                        a_long_static_fld = k5;//c.??
                        if(b_boolean_array1d_static_fld[k6])
                            xd(k6);
                        else
                           // try {
                                avoid1int(k6);
                           // }
                           // catch(Exception exception3) { }
                        if(k6 == 239) {//==
                            repaint();
                            serviceRepaints();
                        }
                        l6 = SystemcurrentTimeMillis();
                        b_long_static_fld += l6 - l9;
                        if((D & 0x7000) == 28672) {
                            D &= 0x8fff;
                            if((D & 0x3e0) == 928) {
                                D ^= 0x800;
                                D &= 0xfc1f;
                                continue;
                            }
                            if((D & 0x3e0) == 992)
                                D &= 0xfc1f;
                            else
                                D += 32;
                        } else {
                            D += 4096;
                        }
                    } else {
                        xd(k6);
                    }
                }

                k5++;
                if(l_boolean_static_fld)
                    k_int_static_fld = (k_int_static_fld & 0xfe) | (k5 & 1);
                if(j_boolean_static_fld)
                    k_int_static_fld = (k_int_static_fld & 0xfd) | (k5 & 1) << 1;
                if(flag && k5 == 0) {
                    i7 = 0;
                    flag = false;
                    for(j7 = 1; j7 < 100; j7++)
                        if(ai1[j7] > ai1[i7])
                            i7 = j7;

                    if(ai1[i7] < 8000)
                        j4 = 0x7fffffff;
                    else
                        j4 = ai[i7];
                    c_long_static_fld = b_long_static_fld = 0L;
                    l6 = SystemcurrentTimeMillis();
                }
                if(p_boolean_static_fld) {
                    long l7 = SystemcurrentTimeMillis();
                    c_long_static_fld += l7 - l6;
            //        while(p_boolean_static_fld) 
            //            try {
            //                Thread.sleep(100L);
            //            }
            //            catch(InterruptedException interruptedexception) { }
                    l6 = SystemcurrentTimeMillis();
                }
//            } while(!v_boolean_static_fld);
            v_boolean_static_fld = false;
            if(n_boolean_static_fld)
                return;
#if defined(SAVE_LOAD)
            if(d_boolean_static_fld) {
                d_boolean_static_fld = false;
                
                if(openstream(1)!=NULL) {
//                    ByteArrayOutputStream bytearrayoutputstream = new ByteArrayOutputStream();
                    bytearrayoutputstreamwrite(j1 & 0xff);
                    bytearrayoutputstreamwrite(l1 & 0xff);
                    bytearrayoutputstreamwrite(i2 & 0xff);
                    bytearrayoutputstreamwrite(k2 & 0xff);
                    bytearrayoutputstreamwrite(k1 & 0xff);
                    bytearrayoutputstreamwrite(l2 & 0xff);
                    bytearrayoutputstreamwrite(i3 & 0xff);
                    bytearrayoutputstreamwrite(j3 & 0xff);
                    bytearrayoutputstreamwrite(k3 & 0xff);
                    bytearrayoutputstreamwrite(l3 & 0xff);
                    bytearrayoutputstreamwrite(i4 & 0xff);
                    bytearrayoutputstreamwrite(j2 & 0xff);
                    bytearrayoutputstreamwrite(k4 >> 0 & 0xff);
                    bytearrayoutputstreamwrite(k4 >> 8 & 0xff);
                    bytearrayoutputstreamwrite(i5 >> 0 & 0xff);
                    bytearrayoutputstreamwrite(i5 >> 8 & 0xff);
                    bytearrayoutputstreamwrite(i5 >> 16 & 0xff);
                    bytearrayoutputstreamwrite(i5 >> 24 & 0xff);
                    bytearrayoutputstreamwrite(l4 >> 0 & 0xff);
                    bytearrayoutputstreamwrite(l4 >> 8 & 0xff);
                    bytearrayoutputstreamwrite(l4 >> 16 & 0xff);
                    bytearrayoutputstreamwrite(l4 >> 24 & 0xff);
                    bytearrayoutputstreamwrite(j4 >> 0 & 0xff);
                    bytearrayoutputstreamwrite(j4 >> 8 & 0xff);
                    bytearrayoutputstreamwrite(j4 >> 16 & 0xff);
                    bytearrayoutputstreamwrite(j4 >> 24 & 0xff);
                    bytearrayoutputstreamwrite(i1 & 0xff);
                    bytearrayoutputstreamwrite(p_boolean_static_fld ? 255 : 0);
                    c_out();//bytearrayoutputstream);
                    b_out();//bytearrayoutputstream);
                    closestream();
//                    bytearrayoutputstream.close();
//                    avoid1String1bytes(b_java_lang_String_static_fld + "save" + z + ".dat", bytearrayoutputstream.toByteArray());
                }
//                catch(Exception exception) { }
            }
            if(o_boolean_static_fld) {
                o_boolean_static_fld = false;
                
              if(openstream(0)!=NULL){
  //                  byte abyte1[] = bbytes1String(b_java_lang_String_static_fld + "save" + z + ".dat");
//                    ByteArrayInputStream bytearrayinputstream = null;
//                    if(abyte1 != null)
//                        bytearrayinputstream = new ByteArrayInputStream(abyte1);
                    j1 = bytearrayinputstreamread() & 0xff;
                    l1 = bytearrayinputstreamread() & 0xff;
                    i2 = bytearrayinputstreamread() & 0xff;
                    k2 = bytearrayinputstreamread() & 0xff;
                    k1 = bytearrayinputstreamread() & 0xff;
                    l2 = bytearrayinputstreamread() & 0xff;
                    i3 = bytearrayinputstreamread() & 0xff;
                    j3 = bytearrayinputstreamread() & 0xff;
                    k3 = bytearrayinputstreamread() & 0xff;
                    l3 = bytearrayinputstreamread() & 0xff;
                    i4 = bytearrayinputstreamread() & 0xff;
                    j2 = bytearrayinputstreamread() & 0xff;
                    k4 = bytearrayinputstreamread() & 0xff;
                    k4 |= (bytearrayinputstreamread() & 0xff) << 8;
                    i5 = bytearrayinputstreamread() & 0xff;
                    i5 |= (bytearrayinputstreamread() & 0xff) << 8;
                    i5 |= (bytearrayinputstreamread() & 0xff) << 16;
                    i5 |= (bytearrayinputstreamread() & 0xff) << 24;
                    l4 = bytearrayinputstreamread() & 0xff;
                    l4 |= (bytearrayinputstreamread() & 0xff) << 8;
                    l4 |= (bytearrayinputstreamread() & 0xff) << 16;
                    l4 |= (bytearrayinputstreamread() & 0xff) << 24;
                    j4 = bytearrayinputstreamread() & 0xff;
                    j4 |= (bytearrayinputstreamread() & 0xff) << 8;
                    j4 |= (bytearrayinputstreamread() & 0xff) << 16;
                    j4 |= (bytearrayinputstreamread() & 0xff) << 24;
                    i1 = (byte)bytearrayinputstreamread();
                    p_boolean_static_fld = bytearrayinputstreamread() == 255;
                    b_in();//(bytearrayinputstream);
                    c_in();//(bytearrayinputstream);
                    closestream();
//                    bytearrayinputstream.close();
                }
//                catch(Exception exception1) { }
                
            }
#endif
            if(h_boolean_static_fld) {
                h_boolean_static_fld = false;
                g();
                for(i6 = 0; i6 < 10240; i6++)
                    abyte0[i6] = 0;

                if(b_byte_array1d_static_fld != null) {
                    for(j6 = 0; j6 < 512; j6++)
                        abyte0[6144 + j6] = b_byte_array1d_static_fld[j6];

                }
                avoid();
                j1 = 0;
                l1 = 0;
                i2 = 0;
                j2 = 255;
                k2 = 0;
                k1 = 0;
                l2 = 1;
                i3 = 0;
                j3 = 0;
                k3 = 0;
                l3 = 0;
                i4 = 0;
                k4 = (abyte0[ai2[7] + 65532] & 0xff) | (abyte0[ai2[7] + 65533] & 0xff) << 8;
                i5 = k4 + (l4 = ai2[k4 >> 13]);
                m();
                k_int_static_fld = 0;
                l_boolean_static_fld = j_boolean_static_fld = false;
            }
//        } while(true);
        s_boolean_static_fld = n_boolean_static_fld = false;
    }

    static void xxf(int i1, int j1) {
    int l1,k1,i2,j2,k2,l2,i3,j3,k3,l3,i4;
//label0:
        switch(w) {
        case 5: // '\005'
        case 9: // '\t'
        case 10: // '\n'
        case 12: // '\f'
        case 13: // '\r'
        case 14: // '\016'
        default:
            break;

        case 1: // '\001'
            if((i1 & 0x6000) != (X & 0x6000)) {
                M = 0;
                H = 0;
            }
            X = i1;
            if((j1 & 0x80) != 0) {
                M = 0;
                H = 0;
                return;
            }
            if((j1 & 1) != 0)
                H |= 1 << M;
            M++;
            if(M < 5)
                return;
            k1 = (i1 & 0x7fff) >> 13;
            d_int_array1d_static_fld[k1] = H;
            M = 0;
            l1 = H;
            H = 0;
            switch(k1) {
            default:
                //break label0;
                goto label0;

            case 0: // '\0'
                if((d_int_array1d_static_fld[0] & 2) != 0) {
                    if((d_int_array1d_static_fld[0] & 1) != 0)
                        avoid4int(0, 0, 1, 1);
                    else
                        avoid4int(0, 1, 0, 1);
                    goto label0;
                    //break label0;
                }
                if((d_int_array1d_static_fld[0] & 1) != 0)
                    avoid4int(1, 1, 1, 1);
                else
                    avoid4int(0, 0, 0, 0);
                goto label0;
                //break label0;

            case 1: // '\001'
                if(U == 1024) {
                    if((d_int_array1d_static_fld[0] & 0x10) != 0) {
                        if(d_int_static_fld != 0) {
                            m_int_static_fld = (d_int_array1d_static_fld[1] & 0x10) >> 4;
                            if((d_int_array1d_static_fld[0] & 8) != 0)
                                m_int_static_fld |= (d_int_array1d_static_fld[2] & 0x10) >> 3;
                            k();
                            d_int_static_fld = 0;
                        } else {
                            d_int_static_fld = 1;
                        }
                    } else {
                        m_int_static_fld = (d_int_array1d_static_fld[1] & 0x10) != 0 ? 3 : 0;
                        k();
                    }
                    goto label0;
                    //break label0;
                }
                if(U == 512 && P == 0) {
                    m_int_static_fld = (d_int_array1d_static_fld[1] & 0x10) >> 4;
                    k();
                    goto label0;
                    //break label0;
                }
                if(P != 0) {
                    if((d_int_array1d_static_fld[0] & 0x10) != 0) {
                        l1 <<= 2;
                        c(0, (l1 + 0) << 10);
                        c(1, (l1 + 1) << 10);
                        c(2, (l1 + 2) << 10);
                        c(3, (l1 + 3) << 10);
                    } else {
                        l1 <<= 2;
                        avoid8int(l1 + 0, l1 + 1, l1 + 2, l1 + 3, l1 + 4, l1 + 5, l1 + 6, l1 + 7);
                    }
                    goto label0;
                    //break label0;
                }
                if((d_int_array1d_static_fld[0] & 0x10) != 0) {
                    l1 <<= 2;
                    bvoid2int(0, l1 + 0);
                    bvoid2int(1, l1 + 1);
                    bvoid2int(2, l1 + 2);
                    bvoid2int(3, l1 + 3);
                }
                goto label0;
                //break label0;

            case 2: // '\002'
                if(U == 1024 && (d_int_array1d_static_fld[0] & 8) != 0) {
                    if(d_int_static_fld != 0) {
                        m_int_static_fld = (d_int_array1d_static_fld[1] & 0x10) >> 4;
                        m_int_static_fld |= (d_int_array1d_static_fld[2] & 0x10) >> 3;
                        k();
                        d_int_static_fld = 0;
                    } else {
                        d_int_static_fld = 1;
                    }
                }
                if(P == 0 && (d_int_array1d_static_fld[0] & 0x10) != 0) {
                    l1 <<= 2;
                    bvoid2int(4, l1 + 0);
                    bvoid2int(5, l1 + 1);
                    bvoid2int(6, l1 + 2);
                    bvoid2int(7, l1 + 3);
                    goto label0;//                    break label0;
                }
                if((d_int_array1d_static_fld[0] & 0x10) != 0) {
                    l1 <<= 2;
                    c(4, (l1 + 0) << 10);
                    c(5, (l1 + 1) << 10);
                    c(6, (l1 + 2) << 10);
                    c(7, (l1 + 3) << 10);
                }
                goto label0;//                break label0;

            case 3: // '\003'
                l1 <<= 1;
                break;
            }
            if((d_int_array1d_static_fld[0] & 8) != 0) {
                if((d_int_array1d_static_fld[0] & 4) != 0) {
                    Z = l1;
                    f_int_static_fld = l1 + 1;
                    n_int_static_fld = T;
                    Q = W;
                } else
                if(U == 1) {
                    Z = 0;
                    f_int_static_fld = 1;
                    n_int_static_fld = l1;
                    Q = l1 + 1;
                }
            } else {
                Z = l1;
                f_int_static_fld = l1 + 1;
                if(U == 1) {
                    n_int_static_fld = l1 + 2;
                    Q = l1 + 3;
                }
            }
            k();
            break;

        case 2: // '\002'
            j1 &= ad - 1;
            xxxxb(j1 * 2, j1 * 2 + 1, ad - 2, ad - 1);
            break;

        case 3: // '\003'
            i2 = (j1 & ((P >> 1) - 1)) << 3;
            avoid8int(i2 + 0, i2 + 1, i2 + 2, i2 + 3, i2 + 4, i2 + 5, i2 + 6, i2 + 7);
            break;

        case 4: // '\004'
            switch(i1 & 0xe001) {
            default:
                break;

            case 32768: 
                d_int_array1d_static_fld[0] = j1;
                l();
                f();
                goto label0; //break label0;

            case 32769: 
                d_int_array1d_static_fld[1] = j1;
                j2 = d_int_array1d_static_fld[1];
                switch(d_int_array1d_static_fld[0] & 7) {
                case 0: // '\0'
                    j2 &= 0xfe;
                    O = j2;
                    l();
                    break;

                case 1: // '\001'
                    j2 &= 0xfe;
                    g_int_static_fld = j2;
                    l();
                    break;

                case 2: // '\002'
                    ac = j2;
                    l();
                    break;

                case 3: // '\003'
                    N = j2;
                    l();
                    break;

                case 4: // '\004'
                    s_int_static_fld = j2;
                    l();
                    break;

                case 5: // '\005'
                    v_int_static_fld = j2;
                    l();
                    break;

                case 6: // '\006'
                    G = j2;
                    f();
                    break;

                case 7: // '\007'
                    J = j2;
                    f();
                    break;
                }
                goto label0; //break label0;

            case 40960: 
                d_int_array1d_static_fld[2] = j1;
                if(f_boolean_static_fld)
                    goto label0; //break label0;
                if((j1 & 1) != 0)
                    avoid4int(0, 0, 1, 1);
                else
                    avoid4int(0, 1, 0, 1);
                break;

            case 40961: 
                d_int_array1d_static_fld[3] = j1;
                q_boolean_static_fld = (j1 & 0x80) == 128;
                break;

            case 49152: 
                d_int_array1d_static_fld[4] = j1;
                I = d_int_array1d_static_fld[4];
                break;

            case 49153: 
                d_int_array1d_static_fld[5] = j1;
                F = d_int_array1d_static_fld[5];
                break;

            case 57344: 
                d_int_array1d_static_fld[6] = j1;
                e_boolean_static_fld = false;
                break;

            case 57345: 
                d_int_array1d_static_fld[7] = j1;
                e_boolean_static_fld = true;
                break;
            }
            break;

        case 6: // '\006'
            k2 = (j1 & 0x3c) >> 2;
            l2 = j1 & 3;
            e(0, (k2 * 2 + 0) << 13);
            e(1, (k2 * 2 + 1) << 13);
            bvoid2int(0, l2 * 8 + 0);
            bvoid2int(1, l2 * 8 + 1);
            bvoid2int(2, l2 * 8 + 2);
            bvoid2int(3, l2 * 8 + 3);
            bvoid2int(4, l2 * 8 + 4);
            bvoid2int(5, l2 * 8 + 5);
            bvoid2int(6, l2 * 8 + 6);
            bvoid2int(7, l2 * 8 + 7);
            break;

        case 7: // '\007'
            i3 = (j1 & 7) << 2;
            xxxxb(i3 + 0, i3 + 1, i3 + 2, i3 + 3);
            if((j1 & 0x10) != 0)
                avoid4int(1, 1, 1, 1);
            else
                avoid4int(0, 0, 0, 0);
            break;

        case 8: // '\b'
            j3 = (j1 & 0xf8) >> 3;
            k3 = j1 & 7;
            e(0, (j3 * 2 + 0) << 13);
            e(1, (j3 * 2 + 1) << 13);
            c(0, (k3 * 8 + 0) << 10);
            c(1, (k3 * 8 + 1) << 10);
            c(2, (k3 * 8 + 2) << 10);
            c(3, (k3 * 8 + 3) << 10);
            c(4, (k3 * 8 + 4) << 10);
            c(5, (k3 * 8 + 5) << 10);
            c(6, (k3 * 8 + 6) << 10);
            c(7, (k3 * 8 + 7) << 10);
            break;

        case 11: // '\013'
            l3 = j1 & 1;
            i4 = (j1 & 0x70) >> 4;
            e(0, (l3 * 4 + 0) << 13);
            e(1, (l3 * 4 + 1) << 13);
            e(2, (l3 * 4 + 2) << 13);
            e(3, (l3 * 4 + 3) << 13);
            c(0, (i4 * 8 + 0) << 10);
            c(1, (i4 * 8 + 1) << 10);
            c(2, (i4 * 8 + 2) << 10);
            c(3, (i4 * 8 + 3) << 10);
            c(4, (i4 * 8 + 4) << 10);
            c(5, (i4 * 8 + 5) << 10);
            c(6, (i4 * 8 + 6) << 10);
            c(7, (i4 * 8 + 7) << 10);
            break;

        case 15: // '\017'
            switch(i1) {
            default:
                goto label0; //break label0;

            case 32768: 
                if((j1 & 0x80) != 0) {
                    e(0, ((j1 & 0x3f) * 2 + 1) << 13);
                    e(1, ((j1 & 0x3f) * 2 + 0) << 13);
                    e(2, ((j1 & 0x3f) * 2 + 3) << 13);
                    e(3, ((j1 & 0x3f) * 2 + 2) << 13);
                } else {
                    e(0, ((j1 & 0x3f) * 2 + 0) << 13);
                    e(1, ((j1 & 0x3f) * 2 + 1) << 13);
                    e(2, ((j1 & 0x3f) * 2 + 2) << 13);
                    e(3, ((j1 & 0x3f) * 2 + 3) << 13);
                }
                if((j1 & 0x40) != 0)
                    avoid4int(0, 0, 1, 1);
                else
                    avoid4int(0, 1, 0, 1);
                goto label0; //break label0;

            case 32769: 
                if((j1 & 0x80) != 0) {
                    e(2, ((j1 & 0x3f) * 2 + 1) << 13);
                    e(3, ((j1 & 0x3f) * 2 + 0) << 13);
                } else {
                    e(2, ((j1 & 0x3f) * 2 + 0) << 13);
                    e(3, ((j1 & 0x3f) * 2 + 1) << 13);
                }
                goto label0; //break label0;

            case 32770: 
                if((j1 & 0x80) != 0) {
                    e(0, ((j1 & 0x3f) * 2 + 1) << 13);
                    e(1, ((j1 & 0x3f) * 2 + 1) << 13);
                    e(2, ((j1 & 0x3f) * 2 + 1) << 13);
                    e(3, ((j1 & 0x3f) * 2 + 1) << 13);
                } else {
                    e(0, ((j1 & 0x3f) * 2 + 0) << 13);
                    e(1, ((j1 & 0x3f) * 2 + 0) << 13);
                    e(2, ((j1 & 0x3f) * 2 + 0) << 13);
                    e(3, ((j1 & 0x3f) * 2 + 0) << 13);
                }
                goto label0; //break label0;

            case 32771: 
                break;
            }
            if((j1 & 0x80) != 0) {
                e(2, ((j1 & 0x3f) * 2 + 1) << 13);
                e(3, ((j1 & 0x3f) * 2 + 0) << 13);
            } else {
                e(2, ((j1 & 0x3f) * 2 + 0) << 13);
                e(3, ((j1 & 0x3f) * 2 + 1) << 13);
            }
            if((j1 & 0x40) != 0)
                avoid4int(0, 0, 1, 1);
            else
                avoid4int(0, 1, 0, 1);
            break;
        }
label0:;
    }

    static void xxd(int i1, int j1) {
        switch(w) {
        case 6: // '\006'
            switch(i1) {
            default:
                break;

            case 17150: 
                if((j1 & 0x10) != 0)
                    avoid4int(1, 1, 1, 1);
                else
                    avoid4int(0, 0, 0, 0);
                break;

            case 17151: 
                if((j1 & 0x10) != 0)
                    avoid4int(0, 0, 1, 1);
                else
                    avoid4int(0, 1, 0, 1);
                break;

            case 17665: 
                e_boolean_static_fld = false;
                break;

            case 17666: 
                I = (I & 0xff00) | (j1 & 0xff);
                break;

            case 17667: 
                I = (I & 0xff) | j1 << 8;
                e_boolean_static_fld = true;
                break;
            }
            break;
        }
    }

    void keyPressed(int i1) {
    if(keyindex<0){
        if(c_boolean_static_fld) {
            if(i1 == key[0])
                k_int_static_fld ^= 0x10;
            else
            if(i1 == key[1])
                k_int_static_fld ^= 0x20;
            else
            if(i1 == key[2])
                k_int_static_fld ^= 0x40;
            else
            if(i1 == key[3])
                k_int_static_fld ^= 0x80;
            else
            if(i1 == key[4])
                k_int_static_fld ^= 1;
            else
            if(i1 == key[5])
                k_int_static_fld ^= 2;
            else
            if(i1 == key[6])
                k_int_static_fld ^= 8;
            else
            if(i1 == key[7])
                k_int_static_fld ^= 4;
            else
            if(i1 == key[8]) {
                k_int_static_fld &= 0xfe;
                l_boolean_static_fld = !l_boolean_static_fld;
            } else
            if(i1 == key[9]) {
                k_int_static_fld &= 0xfd;
                j_boolean_static_fld = !j_boolean_static_fld;
            }
        } else
        if(i1 == key[0])
            k_int_static_fld |= 0x10;
        else
        if(i1 == key[1])
            k_int_static_fld |= 0x20;
        else
        if(i1 == key[2])
            k_int_static_fld |= 0x40;
        else
        if(i1 == key[3])
            k_int_static_fld |= 0x80;
        else
        if(i1 == key[4])
            k_int_static_fld |= 1;
        else
        if(i1 == key[5])
            k_int_static_fld |= 2;
        else
        if(i1 == key[6])
            k_int_static_fld |= 8;
        else
        if(i1 == key[7])
            k_int_static_fld |= 4;
        else
        if(i1 == key[8])
            l_boolean_static_fld = true;
        else
        if(i1 == key[9])
            j_boolean_static_fld = true;
        else
        if(i1 == key[10]) {
            long l1 = SystemcurrentTimeMillis();
            if(l1 - a_long_static_fld < 500L) {
                k_int_static_fld = 0;
                l_boolean_static_fld = j_boolean_static_fld = false;
            }
            a_long_static_fld = l1;
            c_boolean_static_fld = true;
        } else
        if(i1 == key[11]) {
            z = 0;
            v_boolean_static_fld = d_boolean_static_fld = true;
        } else
        if(i1 == key[12]) {
            z = 0;
            v_boolean_static_fld = o_boolean_static_fld = true;
        }
        if(i1 == key[13]) {
            //main.d_boolean_static_fld = !main.d_boolean_static_fld;
            //setFullScreenMode(main.d_boolean_static_fld);
        }
     }else{
		key[keyindex] = i1;
        keyindex=-1;
        
     }
   }

	void keyReleased(int i1) {
        if(c_boolean_static_fld) {
            if(i1 == key[10])
                c_boolean_static_fld = false;
        } else
        if(i1 == key[0])
            k_int_static_fld &= 0xef;
        else
        if(i1 == key[1])
            k_int_static_fld &= 0xdf;
        else
        if(i1 == key[2])
            k_int_static_fld &= 0xbf;
        else
        if(i1 == key[3])
            k_int_static_fld &= 0x7f;
        else
        if(i1 == key[4])
            k_int_static_fld &= 0xfe;
        else
        if(i1 == key[5])
            k_int_static_fld &= 0xfd;
        else
        if(i1 == key[6])
            k_int_static_fld &= 0xf7;
        else
        if(i1 == key[7])
            k_int_static_fld &= 0xfb;
        else
        if(i1 == key[8]) {
            k_int_static_fld &= 0xfe;
            l_boolean_static_fld = false;
        } else
        if(i1 == key[9]) {
            k_int_static_fld &= 0xfd;
            j_boolean_static_fld = false;
        }
    }

    static void j() {
        if(b_int_static_fld < 0)
            b_int_static_fld = 0;
        if(b_int_static_fld > 255)
            b_int_static_fld = 255;
        if(L < 0)
            L = 0;
        if(L > 255)
            L = 255;
        int i1 = 332 + ((b_int_static_fld - 128) * 40) / 256;
        int ai[] = {
            0, 240, 210, 180, 150, 120, 90, 60, 30, 0, 
            330, 300, 270, 0, 0, 0
        };
        int ai1[] = {
            128, 192, 256, 256
        };
        int ai2[] = {
            74, 115, 187, 230
        };
        int ai3[] = {
            0, 61, 120, 197
        };
        for(int j1 = 0; j1 < 4; j1++) {
            for(int l1 = 0; l1 < 16; l1++) {
                int i2 = 0;
                int j2 = 0;
                if(l1 == 0)
                    j2 = ai1[j1];
                else
                if(l1 == 13)
                    j2 = ai3[j1];
                else
                if(l1 != 14 && l1 != 15) {
                    i2 = L;
                    j2 = ai2[j1];
                }
                int k2 = (ai[l1] + i1) % 360;
                int l2 = j2 + aint2int(i2, k2);
                int i3 = j2 - (aint2int(i2, k2) * 27 + xxb(i2, k2) * 10) / 53;
                int j3 = j2 - xxb(i2, k2);
                if((R & 0xe0) != 0)
                    switch(R & 0xe0) {
                    case 32: // ' '
                        i3 = (i3 * 8) / 10;
                        j3 = (j3 * 73) / 100;
                        break;

                    case 64: // '@'
                        l2 = (l2 * 73) / 100;
                        j3 = (j3 * 7) / 10;
                        break;

                    case 96: // '`'
                        l2 = (l2 * 76) / 100;
                        i3 = (i3 * 78) / 100;
                        j3 = (j3 * 58) / 100;
                        break;

                    case 128: 
                        l2 = (l2 * 86) / 100;
                        i3 = (i3 * 8) / 10;
                        break;

                    case 160: 
                        l2 = (l2 * 83) / 100;
                        i3 = (i3 * 68) / 100;
                        j3 = (j3 * 85) / 100;
                        break;

                    case 192: 
                        l2 = (l2 * 67) / 100;
                        i3 = (i3 * 77) / 100;
                        j3 = (j3 * 83) / 100;
                        break;

                    case 224: 
                        l2 = (l2 * 68) / 100;
                        i3 = (i3 * 68) / 100;
                        j3 = (j3 * 68) / 100;
                        break;
                    }
                if(l2 > 255)
                    l2 = 255;
                if(i3 > 255)
                    i3 = 255;
                if(j3 > 255)
                    j3 = 255;
                if(l2 < 0)
                    l2 = 0;
                if(i3 < 0)
                    i3 = 0;
                if(j3 < 0)
                    j3 = 0;
                int k3 = (j1 << 4) + l1;
                g_int_array1d_static_fld[k3] = (l2 & 0xf8)<<8 | (i3 & 0xfc)<<3 | (j3 & 0xf8)>>3;
                  //l2 << 16 | i3 << 8 | j3;
            }

        }

        for(int k1 = 0; k1 < 32; k1++)
            j_int_array1d_static_fld[k1] = g_int_array1d_static_fld[e_int_array1d_static_fld[k1]] | (k1 & 3) << 24;

    }

    static void xd(int i1) {
        byte byte0 = (byte)((S & 0x20) != 0 ? 16 : 8);
        int j1 = i1 - ((d_byte_array1d_static_fld[0] & 0xff) + 1);
        if(j1 >= 0 && j1 < byte0) {
            int k1 = d_byte_array1d_static_fld[1] & 0xff;
            int l1 = d_byte_array1d_static_fld[2] & 0xff;
            boolean flag = (l1 & 0x80) != 0;
            l1 = (l1 & 3) << 2 | 0x10;
            int i2;
            if((S & 0x20) != 0)
                i2 = l_int_array1d_static_fld[((k1 & 1) << 2) + (k1 >> 6)] + ((k1 & 0x3e) << 3);
            else
                i2 = l_int_array1d_static_fld[V + (k1 >> 6)] + ((k1 & 0x3f) << 3);
            ushort c1;
            if(flag)
                c1 = a_char_array1d_static_fld[i2 | (byte0 - 1 - j1)];
            else
                c1 = a_char_array1d_static_fld[i2 | j1];
            if(c1 != 0)
                o_int_static_fld |= 0x40;
        }
    }

    static void h() {
        int i1 = getWidth;
        int j1 = getHeight;
        if(i1 != t_int_static_fld || j1 != K)
            i();
        t_int_static_fld = i1;
        K = j1;
    }

    static boolean bbool() {
        return (d_int_array1d_static_fld[0] & 0x80) != 0;
    }

    static void m() {
        memset(screens,0,screen_length<<1);
//        for(int i1 = 0; i1 < screen_length; i1++)
//            screen[i1] = 0;

    }

    static int xxb(int i1, int j1) {
        int k1;
        if(j1 < 90)
            k1 = a_short_array1d_static_fld[90 - j1];
        else
        if(j1 < 180)
            k1 = -a_short_array1d_static_fld[j1 - 90];
        else
        if(j1 < 270)
            k1 = -a_short_array1d_static_fld[270 - j1];
        else
            k1 = a_short_array1d_static_fld[j1 - 270];
        return (k1 * i1) / 1000;
    }

    boolean loadrom(char *s) {
        int i1 = loadROM(s);
  //savedebug();        
        if(i1 != 0)
            return i1;
        g();
        for(int j1 = 0; j1 < b_int_array1d_static_fld_length; j1++)
            if(j1 == 3)
                b_int_array1d_static_fld[j1] = 2048 - (j1 << 13);
            else
                b_int_array1d_static_fld[j1] = -(j1 << 13);

        avoid();
        b_java_lang_String_static_fld = s;
//savedebug(); 
        //savefile("1.out",vram,0x10810);
        return 0;//true;
    }
#if 0
    static void d() {
        if(s_boolean_static_fld) {
            v_boolean_static_fld = n_boolean_static_fld = true;
     //       try {
     //           Thread.yield();
     //           Thread.sleep(100L);
     //       }
     //       catch(InterruptedException interruptedexception) { }
        }
    }
#endif
    static void c0() {
        h();
        int i1,i2,i3,i4,i5,i6,i7,i8,i9,i10;
        int j1,j2,j3,j4,j5,j6,j7,j8,j9,j10;
        int k1,k2,k3,k4,k5,k6,k7,k8,k9,k10;
        int l1,l2,l3,l4,l5,l6,l7,l8,l9,l10;

        ushort *ai = screens;
        byte *abyte0 = vram;
        byte *abyte1 = d_byte_array1d_static_fld;
        boolean *aflag = a_boolean_array1d_static_fld;
        boolean *aflag1 = b_boolean_array1d_static_fld;
        ushort *ac1 = a_char_array1d_static_fld;
        ushort *ai1 = j_int_array1d_static_fld;
        boolean flag = (R & 2) != 0;
        boolean flag1 = (R & 4) != 0;
        i1 = r_int_static_fld;
        j1 = h_int_static_fld;
        D = y;
        if((R & 8) != 0) {
            for(k1 = 0; k1 < 240;) {
                l1 = D;
                j2 = 0;
                l2 = 0;
                j3 = i_int_array1d_static_fld[8 + (l1 >> 10 & 3)];
                l3 = j3 + (l1 & 0x3ff);
                j4 = l1 >> 12 & 7;
                l2 = abyte0[ram+(j3 + 960 + (l1 >> 4 & 0x38) + (l1 >> 2 & 7))] & 0xff;
                k4 = (l1 & 2) | (l1 >> 4 & 4);
                j2 = (l2 >> k4 & 3) << 2;
                l4 = -c_int_static_fld;
                do {
                    i5 = abyte0[ram+l3] & 0xff;
                    l5 = l_int_array1d_static_fld[e_int_static_fld + (i5 >> 6)] + ((i5 & 0x3f) << 3) + j4;
                    i6 = l4;
                    k6 = (8 + k1) - j4;
                    if(i6 < 0)
                        i6 = 0;
                    i7 = c_int_array1d_static_fld[k1] + f_int_array1d_static_fld[l4 + 7];
                    for(k7 = k1; k7 < k6;) {
                        if(!aflag1[k7]) {
                            i8 = ac1[l5];
                            l8 = i7;
                            for(k9 = l4 + 7; k9 >= i6; k9--) {
                                if(!aflag[k9]) {
                                  if(flag || k9 >= 8){
                                      //if(k9>4) //my add = ничего не выводит тогда
                                        ai[l8] = ai1[j2 | (i8 & 3)];
                                        //if((j2 | i8 & 3)!=0) printf("%d ", j2 | i8 & 3);
                                  }
                                    l8 -= i1;
                                }
                                i8 >>= 2;
                            }

                            i7 += j1;
                        }
                        k7++;
                        l5++;
                    }

                    if((l1 & 0x20) == 0) {
                        j8 = abyte0[ram+(l3 | 0x20)] & 0xff;
                        i9 = l_int_array1d_static_fld[e_int_static_fld + (j8 >> 6)] + ((j8 & 0x3f) << 3);
                        for(k6 += 8; k7 < k6;) {
                            if(!aflag1[k7]) {
                                l9 = ac1[i9];
                                j10 = i7;
                                for(l10 = l4 + 7; l10 >= i6; l10--) {
                                    if(!aflag[l10]) {
                                      if(flag || l10 >= 8){
                                          //if(l10<=3)//my add = no thing
                                            ai[j10] = ai1[j2 | (l9 & 3)];
                                            //if(++nnn>2000000){
                                            // savescreen("out.bmp");
                                              //nnn=0;
                                            //}
                                      }
                                        j10 -= i1;
                                    }
                                    l9 >>= 2;
                                }

                                i7 += j1;
                            }
                            k7++;
                            i9++;
                        }

                    }
                    if((l4 += 8) >= 256)
                        break;
                    l1++;
                    l3++;
                    if((l1 & 1) == 0) {
                        if((l1 & 3) == 0) {
                            if((l1 & 0x1f) == 0) {
                                l1 = (l1 - 32) ^ 0x400;
                                j3 = i_int_array1d_static_fld[8 + (l1 >> 10 & 3)];
                                l3 = j3 + (l1 & 0x3ff);
                            }
                            l2 = abyte0[ram+(j3 + 960 + (l1 >> 4 & 0x38) + (l1 >> 2 & 7))] & 0xff;
                            k4 -= 2;
                        } else {
                            k4 += 2;
                        }
                        j2 = (l2 >> k4 & 3) << 2;
                    }
                } while(true);
                j5 = 8 - j4;
                if((l1 & 0x20) == 0)
                    j5 += 8;
                k1 += j5;
                while(j5 > 0)  {
                    if((D & 0x7000) == 28672) {
                        D &= 0x8fff;
                        if((D & 0x3e0) == 928) {
                            D ^= 0x800;
                            D &= 0xfc1f;
                        } else
                        if((D & 0x3e0) == 992)
                            D &= 0xfc1f;
                        else
                            D += 32;
                    } else {
                        D += 4096;
                    }
                    j5--;
                }
            }

        }
        if((R & 0x10) != 0) {
            byte byte0 = (byte)((S & 0x20) != 0 ? 16 : 8);
//label0:
            for(i2 = 252; i2 >= 0; i2 -= 4) {
               k2 = (abyte1[i2] & 0xff) + 1;
                i3 = abyte1[i2 + 1] & 0xff;
                k3 = abyte1[i2 + 2] & 0xff;
                i4 = abyte1[i2 + 3] & 0xff;
                if(k2 >= 240)
                    continue;
                boolean flag2 = (k3 & 0x80) != 0;
                boolean flag3 = (k3 & 0x40) != 0;
                boolean flag4 = (k3 & 0x20) != 0;
                k3 = (k3 & 3) << 2 | 0x10;
                k5 = c_int_array1d_static_fld[k2];
                if(byte0 == 16)
                    j6 = l_int_array1d_static_fld[((i3 & 1) << 2) + (i3 >> 6)] + ((i3 & 0x3e) << 3);
                else
                    j6 = l_int_array1d_static_fld[V + (i3 >> 6)] + ((i3 & 0x3f) << 3);
                l6 = byte0;
                j7 = k2;
                do {
                    if(l6 <= 0)
                        goto label0;//continue label0;
                    if(!aflag1[j7]) {
                        if(flag2)
                            l7 = ac1[j6 | (l6 - 1)];
                        else
                            l7 = ac1[j6 | (byte0 - l6)];
                        if(flag3)
                            k8 = i4;
                        else
                            k8 = i4 + 7;
                        if(flag3)
                            j9 = k5 + h_int_array1d_static_fld[k8];
                        else
                            j9 = k5 + f_int_array1d_static_fld[k8];
                        for(i10 = 0; i10 < 8; i10++) {
                            if(!aflag[k8]) {
                                k10 = l7 & 3;
                                if(k10 != 0 && (flag1 || (k8 >= 8 && k8 < 248)) && (!flag4 || ai[j9] >> 24 == 0))
                                    ai[j9] = ai1[k10 | k3];
                                if(flag3)
                                    j9 += i1;
                                else
                                    j9 -= i1;
                            }
                            l7 >>= 2;
                            if(flag3)
                                k8++;
                            else
                                k8--;
                        }

                        k5 += j1;
                    }
                    l6--;
                    j7++;
                } while(true);
              label0:;/////???
            }

        }
        repaint();
        serviceRepaints();
    }

    static void avoid1int(int i1) {
        h();
        int j1 = c_int_array1d_static_fld[i1];
        ushort *ai = screens;
        byte *abyte0 = d_byte_array1d_static_fld;
        byte *abyte1 = vram;
        ushort *ai1 = j_int_array1d_static_fld;
        boolean flag = (R & 2) != 0;
        boolean flag1 = (R & 4) != 0;
        if((R & 8) != 0) {
            int k1 = D;
            int l1 = 0;
            int j2 = 0;
            int l2 = i_int_array1d_static_fld[8 + (k1 >> 10 & 3)];
            int j3 = k1 >> 12 & 7;
            j2 = abyte1[ram+(l2 + 960 + (k1 >> 4 & 0x38) + (k1 >> 2 & 7))] & 0xff;
            int l3 = (k1 & 2) | (k1 >> 4 & 4);
            l1 = ((j2 >> l3) & 3) << 2;
            int j4 = -c_int_static_fld;
            do {
                int k4 = abyte1[ram+l2 + (k1 & 0x3ff)] & 0xff;
                int l4 = l_int_array1d_static_fld[e_int_static_fld + (k4 >> 6)] + ((k4 & 0x3f) << 3) + j3;
                int i5 = j4;
                int k5 = (8 + i1) - j3;
                if(k5 > 240)
                    k5 = 240;
                if(i5 < 0)
                    i5 = 0;
                int i6 = a_char_array1d_static_fld[l4];
                int k6 = j1 + f_int_array1d_static_fld[j4 + 7];
                for(int i7 = j4 + 7; i7 >= i5; i7--) {
                    if(!a_boolean_array1d_static_fld[i7]) {
                        if(i7 < 256 && (i7 >= 8 || flag))
                            ai[k6] = ai1[l1 | (i6 & 3)];
                        k6 -= r_int_static_fld;
                    }
                    i6 >>= 2;
                }

                if((j4 += 8) >= 256)
                    break;
                if((++k1 & 1) == 0) {
                    if((k1 & 3) == 0) {
                        if((k1 & 0x1f) == 0) {
                            k1 = (k1 - 32) ^ 0x400;
                            l2 = i_int_array1d_static_fld[8 + (k1 >> 10 & 3)];
                        }
                        j2 = abyte1[ram+l2 + 960 + (k1 >> 4 & 0x38) + (k1 >> 2 & 7)] & 0xff;
                        l3 -= 2;
                    } else {
                        l3 += 2;
                    }
                    l1 = (j2 >> l3 & 3) << 2;
                }
            } while(true);
        }
        if((R & 0x10) != 0) {
            byte byte0 = (byte)((S & 0x20) != 0 ? 16 : 8);
            for(int i2 = 0; i2 < 256; i2 += 4) {
                int k2 = i1 - ((abyte0[i2] & 0xff) + 1);
                int i3 = abyte0[i2 + 1] & 0xff;
                int k3 = abyte0[i2 + 2] & 0xff;
                int i4 = abyte0[i2 + 3] & 0xff;
                if(k2 < 0 || k2 >= byte0)
                    continue;
                boolean flag2 = (k3 & 0x80) != 0;
                boolean flag3 = (k3 & 0x40) != 0;
                boolean flag4 = (k3 & 0x20) != 0;
                k3 = (k3 & 3) << 2 | 0x10;
                int j5;
                if(byte0 == 16)
                    j5 = l_int_array1d_static_fld[((i3 & 1) << 2) + (i3 >> 6)] + ((i3 & 0x3e) << 3);
                else
                    j5 = l_int_array1d_static_fld[V + (i3 >> 6)] + ((i3 & 0x3f) << 3);
                int l5;
                if(flag2)
                    l5 = a_char_array1d_static_fld[j5 | (byte0 - 1 - k2)];
                else
                    l5 = a_char_array1d_static_fld[j5 | k2];
                int j6;
                if(flag3)
                    j6 = i4;
                else
                    j6 = i4 + 7;
                int l6 = j1 + h_int_array1d_static_fld[j6];
                if(!flag3)
                    l6 -= a_boolean_array1d_static_fld[j6] ? r_int_static_fld : 0;
                for(int j7 = 0; j7 < 8; j7++) {
                    if(!a_boolean_array1d_static_fld[j6]) {
                        if(j6 < 256) {
                            int k7 = ai[l6] >> 24;
                            if((k7 & 4) == 0) {
                                int l7 = l5 & 3;
                                if(l7 != 0) {
                                    if(i2 == 0 && k7 != 0)
                                        o_int_static_fld |= 0x40;
                                    if(flag1 || (j6 >= 8 && j6 < 248)) {
                                        if(!flag4 || k7 == 0)
                                            ai[l6] = ai1[l7 | k3] | 0x4000000;
                                        else
                                            ai[l6] |= 0x4000000;
                                    }
                                }
                            }
                        }
                        if(flag3)
                            l6 += r_int_static_fld;
                        else
                            l6 -= r_int_static_fld;
                    }
                    l5 >>= 2;
                    if(flag3)
                        j6++;
                    else
                        j6--;
                }

            }

        }
    }
/*
    static String[] aStrings1bool(boolean flag) {
        char *as[] = new String[10];
        for(int i1 = 0; i1 < 10; i1++) {
            if(flag) {
                as[i1] = "Slot " + (i1 + 1) + " Unregistered";
                continue;
            }
            try {
                DataInputStream datainputstream = Connector.openDataInputStream(b_java_lang_String_static_fld + "save" + i1 + ".dat");
                if(datainputstream == null) {
                    as[i1] = "Slot " + (i1 + 1) + "  Empty";
                    continue;
                }
                long l1 = 0L;
                if(l1 >= 60L)
                    as[i1] = "Slot " + (i1 + 1) + " saved " + l1 / 60L + "h" + l1 % 60L + "m before";
                else
                    as[i1] = "Slot " + (i1 + 1) + " saved " + l1 + "m before";
            }
            catch(IOException ioexception) {
                as[i1] = "Slot " + (i1 + 1) + "  Empty";
            }
        }

        return as;
    }
*/
    static void g() {
        S = 0;
        R = 0;
        o_int_static_fld = 0;
        for(int i1 = 0; i1 < 8192; i1++)
            vram[ram+i1] = 0;

        for(int j1 = 0; j1 < 12; j1++)
            if(j1 < 8) {
                i_int_array1d_static_fld[j1] = 8192 + 1024 * j1;
                l_int_array1d_static_fld[j1] = 512 * j1;
            } else {
                i_int_array1d_static_fld[j1] = 0;
            }

        if(f_boolean_static_fld)
            avoid4int(0, 1, 2, 3);
        else
        if(!g_boolean_static_fld)
            avoid4int(0, 0, 1, 1);
        else
        if(g_boolean_static_fld)
            avoid4int(0, 1, 0, 1);
        else
            avoid4int(0, 1, 2, 3);
        a_char_array1d_static_fld = (ushort*)malloc((ramsize / 2 - 4096)*2);
        memset(a_char_array1d_static_fld,0,(ramsize / 2 - 4096)*2);//clr
        for(int k1 = 8192; k1 < ramsize; k1++)
            if((k1 & 8) == 0) {
                int i2 = vram[ram+k1] & 0xff;
                int j2 = vram[ram+(k1 | 8)] & 0xff;
                int k2 = ((k1 & 0xfffffff0) >> 1 | (k1 & 7)) - 4096;
                a_char_array1d_static_fld[k2] = (ushort)(
                    (i2 & 1)         |
                    (i2 & 2) << 1    |
                    (i2 & 4) << 2    |
                    (i2 & 8) << 3    |
                    (i2 & 0x10) << 4 |
                    (i2 & 0x20) << 5 |
                    (i2 & 0x40) << 6 |
                    (i2 & 0x80) << 7 |
                    (j2 & 1) << 1    |
                    (j2 & 2) << 2    |
                    (j2 & 4) << 3    |
                    (j2 & 8) << 4    |
                    (j2 & 0x10) << 5 |
                    (j2 & 0x20) << 6 |
                    (j2 & 0x40) << 7 |
                    (j2 & 0x80) << 8
                );
            }

        C = 0;
        ab = 0;
        for(int l1 = 0; l1 < d_byte_array1d_static_fld_length; l1++)
            d_byte_array1d_static_fld[l1] = 0;

        e_int_static_fld = 0;
        V = 0;
        b_boolean_static_fld = false;
        D = 0;
        y = 0;
        c_int_static_fld = 0;
        q_int_static_fld = 0;
        j_int_static_fld = 0;
        x = 1;
        j();
    }
/*
    byte[] abytes1String(char *s) throws IOException {
        InputStream inputstream = main.getClass().getResourceAsStream(s);
        ByteArrayOutputStream bytearrayoutputstream = new ByteArrayOutputStream();
        int i1;
        while((i1 = inputstream.read()) != -1) 
            bytearrayoutputstream.write(i1);
        byte abyte0[] = bytearrayoutputstream.toByteArray();
        inputstream.close();
        bytearrayoutputstream.close();
        return abyte0;
    }
*/

    static int loadROM(char *s) {
    	int i1=0, filesize, vramlength, loadfilesize;
      char *buf;
      if(!(buf=loadfile(s, &loadfilesize))) return 3;
      filesize=loadfilesize;
      //int f;
      //if((f=fopen(s,A_ReadOnly+A_BIN,P_READ,&err))==-1) return 3;
      //filesize=lseek(f,0,2,&err,&err);
      //lseek(f,0,0, &err,&err);
      if(!(vram = (byte*)malloc(vramlength=filesize+10240+16384))){ mfree(buf); return 3;}
      memset(vram,0,vramlength);
      memcpy(vram,buf,filesize);
      mfree(buf);
//      fread(f, vram,filesize,&err);
//      fclose(f, &err);

      //        bytearrayinputstream = new ByteArrayInputStream(bbytes1String("file:///0:/Misc/" + s + ".nes"));
        if(vramlength<(10243+16384) || !(vram[0] == 78 && vram[1] == 69 && vram[2] == 83 && vram[3] == 26)){
        //a_java_lang_String_static_fld = "rom header error";
        return 4;
        }
//}catch(IOException ioes){return 0;}
//        try {
            u_int_static_fld = vram[4];
            i_int_static_fld = vram[5];
            i1 = vram[6];
            g_boolean_static_fld = (i1 & 1) != 0;
            a_boolean_static_fld = (i1 & 2) != 0;
            r_boolean_static_fld = (i1 & 4) != 0;
            f_boolean_static_fld = (i1 & 8) != 0;
            int k1 = i1 >> 4 & 0xf;
            int l2 = vram[7] & 0xf0;
            w = k1 | l2;
            for(int i3 = 0; i3 < 8; i3++)
                if(vram[i3+8] != 0)
                    w &= 0xf;

           // break MISSING_BLOCK_LABEL_232;
//        } catch(Exception exception) {
//            a_java_lang_String_static_fld = exception.toString();
//            return 20;
//        }
        
        switch(w){
        case 5: // '\005'
        case 9: // '\t'
        case 10: // '\n'
        case 12: // '\f'
        case 13: // '\r'
        case 14: // '\016'
        default:
          //  a_java_lang_String_static_fld = "unsupported mapper " + w;
            return 5;

        case 0: // '\0'
        case 1: // '\001'
        case 2: // '\002'
        case 3: // '\003'
        case 4: // '\004'
        case 6: // '\006'
        case 7: // '\007'
        case 8: // '\b'
        case 11: // '\013'
        case 15: // '\017'
            break;
		}
//try{//add		
        if(u_int_static_fld == 0){
//	        a_java_lang_String_static_fld = "no program rom found";
    	    return 6;
    	}
        Y = 16384 * u_int_static_fld;
//        int j1 = Y + 10240;
        //vram = new byte[j1];
        Systemarraycopy(vram,0,vram,10240,vramlength-10240);
        ram=10256;
        if(r_boolean_static_fld) {
            b_byte_array1d_static_fld = (byte*)malloc(512);
            for(int l1 = 0; l1 < 512; l1++)
                b_byte_array1d_static_fld[l1] = vram[6144 + l1] = vram[ram++];
        } else {
            b_byte_array1d_static_fld = null;
            for(int i2 = 0; i2 < 512; i2++)
                vram[6144 + i2] = 0;

        }
        //for(int j2 = 0; j2 < Y; j2++)
        //    vram[j2 + 10240] = (byte)bytearrayinputstream.read();
        Systemarraycopy(vram,ram,vram,10240,Y);
        ram+=Y;
        E = 8192 * i_int_static_fld;
        if(E == 0)
            ramsize = 16384;
        else{
            ramsize = E + 8192;
        //a_byte_array1d_static_fld = new byte[ramsize];
        //for(int k2 = 0; k2 < E; k2++)
        //    a_byte_array1d_static_fld[k2 + 8192] = (byte)bytearrayinputstream.read();
        	Systemarraycopy(vram,ram,vram,ram+8192,E);
       	}
//}catch(Exception exception1){
//        a_java_lang_String_static_fld = exception1.toString();
//        return 20;
//        }
        a_java_lang_String_static_fld = null;
        return 0;
    }
/*
    static void avoid1InputStream(InputStream inputstream) throws IOException {
        //c c1 = main;
        d_boolean_static_fld = inputstream.read() == 255;//c.??
        u_boolean_static_fld = inputstream.read() == 255;
        m_boolean_static_fld = inputstream.read() == 255;
        t_boolean_static_fld = inputstream.read() == 255;
        k_boolean_static_fld = inputstream.read() == 255;
        B = inputstream.read();
        A = inputstream.read();
        b_int_static_fld = inputstream.read();
        L = inputstream.read();
        //c c2 = main;
        f_boolean_static_fld = inputstream.read() == 255;//c.??
        inputstream.read();
        inputstream.read();
        inputstream.read();
        inputstream.read();
        main.b_int_fld = inputstream.read();
        main.b_int_fld |= inputstream.read() << 8;
        main.e_boolean_fld = inputstream.read() == 255;
        if(main.e_boolean_fld)
        ;
//            main.f_javax_microedition_lcdui_List_fld.set(3, "Help", null);
        int i1 = inputstream.read();
        main.c_java_lang_String_fld = "";
        for(int j1 = 0; j1 < i1; j1++)
            main.c_java_lang_String_fld += inputstream.read();

        for(int k1 = 0; k1 < key.length; k1++)
            key[k1] = (byte)inputstream.read();

    }
*/
    static void avoid() {
        switch(w) {
        case 1: // '\001'
            //d_int_array1d_static_fld = new int[4];
            break;

        case 4: // '\004'
            //d_int_array1d_static_fld = new int[8];
            break;
        }
        ad = Y >> 13;
        P = E >> 10;
        b();
    }

    static void b() {
        int i1,j1;
        switch(w) {
        case 5: // '\005'
        case 9: // '\t'
        case 10: // '\n'
        case 12: // '\f'
        case 13: // '\r'
        case 14: // '\016'
        default:
            break;

        case 0: // '\0'
            if(ad > 2)
                xxxxb(0, 1, 2, 3);
            else
            if(ad > 1)
                xxxxb(0, 1, 0, 1);
            else
                xxxxb(0, 0, 0, 0);
            if(P > 0)
                avoid8int(0, 1, 2, 3, 4, 5, 6, 7);
            break;

        case 1: // '\001'
            M = 0;
            H = 0;
            d_int_array1d_static_fld[0] = 12;
            d_int_array1d_static_fld[1] = 0;
            d_int_array1d_static_fld[2] = 0;
            d_int_array1d_static_fld[3] = 0;
            i1 = ad * 8;
            if(i1 == 1024)
                U = 1024;
            else
            if(i1 == 512)
                U = 512;
            else
                U = 1;
            m_int_static_fld = 0;
            d_int_static_fld = 0;
            if(U == 1) {
                T = ad - 2;
                W = ad - 1;
            } else {
                T = 30;
                W = 31;
            }
            Z = 0;
            f_int_static_fld = 1;
            n_int_static_fld = T;
            Q = W;
            k();
            avoid8int(0, 1, 2, 3, 4, 5, 6, 7);
            break;

        case 2: // '\002'
            xxxxb(0, 1, ad - 2, ad - 1);
            avoid8int(0, 1, 2, 3, 4, 5, 6, 7);
            break;

        case 3: // '\003'
            if(ad > 2)
                xxxxb(0, 1, 2, 3);
            else
                xxxxb(0, 1, 0, 1);
            avoid8int(0, 1, 2, 3, 4, 5, 6, 7);
            break;

        case 4: // '\004'
            i_boolean_static_fld = true;
            for(j1 = 0; j1 < 8; j1++)
                d_int_array1d_static_fld[j1] = 0;

            G = 0;
            J = 1;
            f();
            if(P > 0) {
                O = 0;
                g_int_static_fld = 2;
                ac = 4;
                N = 5;
                s_int_static_fld = 6;
                v_int_static_fld = 7;
                l();
            } else {
                O = g_int_static_fld = ac = N = s_int_static_fld = v_int_static_fld = 0;
            }
            e_boolean_static_fld = false;
            I = 0;
            F = 0;
            avoid8int(0, 1, 2, 3, 4, 5, 6, 7);
            break;

        case 6: // '\006'
            i_boolean_static_fld = true;
            xxxxb(0, 1, 14, 15);
            if(P > 0) {
                avoid8int(0, 1, 2, 3, 4, 5, 6, 7);
            } else {
                bvoid2int(0, 0);
                bvoid2int(0, 1);
                bvoid2int(0, 2);
                bvoid2int(0, 3);
                bvoid2int(0, 4);
                bvoid2int(0, 5);
                bvoid2int(0, 6);
                bvoid2int(0, 7);
            }
            break;

        case 7: // '\007'
            xxxxb(0, 1, 2, 3);
            break;

        case 8: // '\b'
            xxxxb(0, 1, 2, 3);
            avoid8int(0, 1, 2, 3, 4, 5, 6, 7);
            break;

        case 11: // '\013'
            xxxxb(0, 1, 2, 3);
            avoid8int(0, 1, 2, 3, 4, 5, 6, 7);
            avoid4int(0, 1, 0, 1);
            break;

        case 15: // '\017'
            xxxxb(0, 1, 2, 3);
            break;
        }
    }

    static void c_in(){//(InputStream inputstream) throws IOException {
        switch(w) {
        default:
            break;

        case 1: // '\001'
            X = (inputstreamread() & 0xff) << 8;
            X |= inputstreamread() & 0xff;
            M = inputstreamread() & 0xff;
            H = inputstreamread() & 0xff;
            for(int i1 = 0; i1 < d_int_array1d_static_fld_length; i1++)
                d_int_array1d_static_fld[i1] = inputstreamread() & 0xff;

            Z = inputstreamread() & 0xff;
            f_int_static_fld = inputstreamread() & 0xff;
            n_int_static_fld = inputstreamread() & 0xff;
            Q = inputstreamread() & 0xff;
            U = inputstreamread() & 0xff;
            m_int_static_fld = inputstreamread() & 0xff;
            d_int_static_fld = inputstreamread() & 0xff;
            T = inputstreamread() & 0xff;
            W = inputstreamread() & 0xff;
            k();
            break;

        case 4: // '\004'
            I = inputstreamread() & 0xff;
            e_boolean_static_fld = inputstreamread() == 255;
            F = inputstreamread() & 0xff;
            for(int j1 = 0; j1 < d_int_array1d_static_fld_length; j1++)
                d_int_array1d_static_fld[j1] = inputstreamread() & 0xff;

            G = inputstreamread() & 0xff;
            J = inputstreamread() & 0xff;
            O = inputstreamread() & 0xff;
            g_int_static_fld = inputstreamread() & 0xff;
            ac = inputstreamread() & 0xff;
            N = inputstreamread() & 0xff;
            s_int_static_fld = inputstreamread() & 0xff;
            v_int_static_fld = inputstreamread() & 0xff;
            break;

        case 6: // '\006'
            I = (inputstreamread() << 8) + inputstreamread();
            e_boolean_static_fld = inputstreamread() == 255;
            break;
        }
    }

    static void b_out(){//(OutputStream outputstream) throws IOException {
        switch(w) {
        default:
            break;

        case 1: // '\001'
            outputstreamwrite(X >> 8 & 0xff);
            outputstreamwrite(X & 0xff);
            outputstreamwrite(M & 0xff);
            outputstreamwrite(H & 0xff);
            for(int i1 = 0; i1 < d_int_array1d_static_fld_length; i1++)
                outputstreamwrite(d_int_array1d_static_fld[i1] & 0xff);

            outputstreamwrite(Z & 0xff);
            outputstreamwrite(f_int_static_fld & 0xff);
            outputstreamwrite(n_int_static_fld & 0xff);
            outputstreamwrite(Q & 0xff);
            outputstreamwrite(U & 0xff);
            outputstreamwrite(m_int_static_fld & 0xff);
            outputstreamwrite(d_int_static_fld & 0xff);
            outputstreamwrite(T & 0xff);
            outputstreamwrite(W & 0xff);
            break;

        case 4: // '\004'
            outputstreamwrite(I & 0xff);
            outputstreamwrite(e_boolean_static_fld ? 255 : 0);
            outputstreamwrite(F & 0xff);
            for(int j1 = 0; j1 < d_int_array1d_static_fld_length; j1++)
                outputstreamwrite(d_int_array1d_static_fld[j1] & 0xff);

            outputstreamwrite(G & 0xff);
            outputstreamwrite(J & 0xff);
            outputstreamwrite(O & 0xff);
            outputstreamwrite(g_int_static_fld & 0xff);
            outputstreamwrite(ac & 0xff);
            outputstreamwrite(N & 0xff);
            outputstreamwrite(s_int_static_fld & 0xff);
            outputstreamwrite(v_int_static_fld & 0xff);
            break;

        case 6: // '\006'
            outputstreamwrite((I & 0xff00) >> 8);
            outputstreamwrite(I & 0xff);
            outputstreamwrite(e_boolean_static_fld ? 255 : 0);
            break;
        }
    }

    static boolean abool() {
        return (d_int_array1d_static_fld[0] & 0x40) != 0;
    }

    static int aint1int(int i1) {
        int j1,k1;
        switch(i1 >> 13) {
        case 0: // '\0'
            return vram[i1 & 0x7ff] & 0xff;

        case 1: // '\001'
            switch(i1 & 7) {
            case 2: // '\002'
                b_boolean_static_fld = false;
                j1 = o_int_static_fld;
 //debug("aint1",j1,j_int_static_fld);                
                o_int_static_fld &= 0x7f;
                j_int_static_fld = (j1 & 0xe0) | (j_int_static_fld & 0x1f);
//    debug("aint2",j1,j_int_static_fld);
                return j_int_static_fld;

            case 7: // '\007'
                j_int_static_fld = q_int_static_fld;
                i1 = D & 0x3fff;
                D += x;
                if(i1 >= 12288)
                    i1 &= 0xefff;
                q_int_static_fld = vram[ram+i_int_array1d_static_fld[i1 >> 10] + (i1 & 0x3ff)] & 0xff;
                return j_int_static_fld;
            }
            return j_int_static_fld;

        case 2: // '\002'
            if(i1 == 16406) {
                k1 = a_int_static_fld & 1;
                a_int_static_fld >>= 1;
                return k1;
            }
            if(i1 < 16408)
                return 0;
            else
                return i1 >> 8;

        case 3: // '\003'
            return vram[i1 - 22528] & 0xff;
        }
        return vram[b_int_array1d_static_fld[i1 >> 13] + i1] & 0xff;
    }
/*
    byte[] bbytes1String(char *s) throws IOException {

        DataInputStream datainputstream = Connector.openDataInputStream(s);
        int i1 = datainputstream.available();
        if(i1 < 3) {
            throw new IOException();
        } else {
            byte abyte0[] = new byte[i1];
            datainputstream.read(abyte0);
            datainputstream.close();
            return abyte0;
        }
  
    InputStream in = getClass().getResourceAsStream("/1"); 
    byte a[] = new byte[in.available()];
    in.read(a);
    in.close();
	return a;
	}
*/
    static void i() {
        if(m_boolean_static_fld) {
            aa = (getHeight * B) / 100;
            if(u_boolean_static_fld)
                l_int_static_fld = (aa * 240) / 256;
            else
                l_int_static_fld = (getWidth * B) / 100;
            if(aa > 256)
                aa = 256;
            if(l_int_static_fld > 240)
                l_int_static_fld = 240;
            if(aa < 64)
                aa = 64;
            if(l_int_static_fld < 60)
                l_int_static_fld = 60;
            //main_a_int_array1d_fld = 
            if(screens) mfree(screens);
            Systemgc();
            //main_a_int_array1d_fld = 
              screens = (ushort*)malloc(l_int_static_fld * aa*2);
            for(int i1 = 0; i1 < h_int_array1d_static_fld_length; i1++)
                h_int_array1d_static_fld[i1] = (((256 - i1 - 1) * aa) / 256) * l_int_static_fld;

            for(int j1 = 0; j1 < c_int_array1d_static_fld_length; j1++)
                c_int_array1d_static_fld[j1] = (j1 * l_int_static_fld) / 240;

            r_int_static_fld = -l_int_static_fld;
            h_int_static_fld = 1;
        } else {
            l_int_static_fld = (getWidth * B) / 100;
            if(u_boolean_static_fld)
                aa = (l_int_static_fld * 240) / 256;
            else
                aa = (getHeight * B) / 100;
            if(l_int_static_fld > 256)
                l_int_static_fld = 256;
            if(aa > 240)
                aa = 240;
            if(l_int_static_fld < 64)
                l_int_static_fld = 64;
            if(aa < 60)
                aa = 60;
            //main_a_int_array1d_fld = 
              if(screens) mfree(screens);
            Systemgc();
            //main_a_int_array1d_fld = 
              screens = (ushort*)malloc(l_int_static_fld * aa*2);
            for(int k1 = 0; k1 < h_int_array1d_static_fld_length; k1++)
                h_int_array1d_static_fld[k1] = (k1 * l_int_static_fld) / 256;

            for(int l1 = 0; l1 < c_int_array1d_static_fld_length; l1++)
                c_int_array1d_static_fld[l1] = ((l1 * aa) / 240) * l_int_static_fld;

            r_int_static_fld = 1;
            h_int_static_fld = l_int_static_fld;
        }
        for(int i2 = 256; i2 < h_int_array1d_static_fld_length; i2++)
            h_int_array1d_static_fld[i2] = h_int_array1d_static_fld[255];

        for(int j2 = 240; j2 < c_int_array1d_static_fld_length; j2++)
            c_int_array1d_static_fld[j2] = c_int_array1d_static_fld[239];

        for(int k2 = 0; k2 < h_int_array1d_static_fld_length; k2++) {
            a_boolean_array1d_static_fld[k2] = k2 < h_int_array1d_static_fld_length - 1 && h_int_array1d_static_fld[k2] == h_int_array1d_static_fld[k2 + 1];
            f_int_array1d_static_fld[k2] = h_int_array1d_static_fld[k2] - (a_boolean_array1d_static_fld[k2] ? r_int_static_fld : 0);
        }

        for(int l2 = 0; l2 < c_int_array1d_static_fld_length; l2++)
            b_boolean_array1d_static_fld[l2] = l2 < c_int_array1d_static_fld_length - 1 && c_int_array1d_static_fld[l2] == c_int_array1d_static_fld[l2 + 1];

        main_a_int_fld = l_int_static_fld;
        main_c_int_fld = aa;
        main_b_boolean_fld = true;
    }

/*
    static void avoid1OutputStream(OutputStream outputstream) throws IOException {
        //c c1 = main;
        outputstream.write(d_boolean_static_fld ? 255 : 0);//c.??
        outputstream.write(u_boolean_static_fld ? 255 : 0);
        outputstream.write(m_boolean_static_fld ? 255 : 0);
        outputstream.write(t_boolean_static_fld ? 255 : 0);
        outputstream.write(k_boolean_static_fld ? 255 : 0);
        outputstream.write(B);
        outputstream.write(A);
        outputstream.write(b_int_static_fld);
        outputstream.write(L);
        //c c2 = main;
        outputstream.write(f_boolean_static_fld ? 255 : 0);
        outputstream.write(0);
        outputstream.write(0);
        outputstream.write(0);
        outputstream.write(0);
        outputstream.write(main.b_int_fld & 0xff);
        outputstream.write(main.b_int_fld >> 8 & 0xff);
        outputstream.write(main.e_boolean_fld ? 255 : 0);
        outputstream.write(main.c_java_lang_String_fld.length());
        for(int i1 = 0; i1 < main.c_java_lang_String_fld.length(); i1++)
            outputstream.write(main.c_java_lang_String_fld.charAt(i1) - 48);

        for(int j1 = 0; j1 < key.length; j1++)
            outputstream.write(key[j1]);

    }
*/
    static void e(int i1, int j1) {
        j1 %= Y;
        b_int_array1d_static_fld[i1 + 4] = (j1 - ((i1 + 4) << 13)) + 10240;
    }

    static void xxxxb(int i1, int j1, int k1, int l1) {
        e(0, i1 << 13);
        e(1, j1 << 13);
        e(2, k1 << 13);
        e(3, l1 << 13);
    }

    static void avoid4int(int i1, int j1, int k1, int l1) {
        i1 &= 3;
        j1 &= 3;
        k1 &= 3;
        l1 &= 3;
        i_int_array1d_static_fld[8] = 0 + (i1 << 10);
        i_int_array1d_static_fld[9] = 0 + (j1 << 10);
        i_int_array1d_static_fld[10] = 0 + (k1 << 10);
        i_int_array1d_static_fld[11] = 0 + (l1 << 10);
    }

    static void c(int i1, int j1) {
        if(E == 0) {
            return;
        } else {
            j1 %= E;
            i_int_array1d_static_fld[i1 & 7] = j1 + 8192;
            l_int_array1d_static_fld[i1 & 7] = j1 >> 1;
            return;
        }
    }

    static void avoid8int(int i1, int j1, int k1, int l1, int i2, int j2, int k2, int l2) {
        c(0, i1 << 10);
        c(1, j1 << 10);
        c(2, k1 << 10);
        c(3, l1 << 10);
        c(4, i2 << 10);
        c(5, j2 << 10);
        c(6, k2 << 10);
        c(7, l2 << 10);
    }

    static void bvoid2int(int i1, int j1) {
        if(i1 < 8) {
            i_int_array1d_static_fld[i1] = 8192 + ((j1 & 0xf) << 10);
            l_int_array1d_static_fld[i1] = (j1 & 0xf) << 9;
        } else
        if(i1 < 12)
            i_int_array1d_static_fld[i1] = 0 + ((j1 & 3) << 10);
    }

    static int aint2int(int i1, int j1) {
        int k1;
        if(j1 < 90)
            k1 = a_short_array1d_static_fld[j1];
        else
        if(j1 < 180)
            k1 = a_short_array1d_static_fld[180 - j1];
        else
        if(j1 < 270)
            k1 = -a_short_array1d_static_fld[j1 - 180];
        else
            k1 = -a_short_array1d_static_fld[360 - j1];
        return (k1 * i1) / 1000;
    }

    static void b_in(){//(InputStream inputstream) throws IOException {
        for(int i1 = 0; i1 < 10240; i1++)
            vram[i1] = (byte)inputstreamread();

        for(int j1 = 0; j1 < b_int_array1d_static_fld_length; j1++) {
            b_int_array1d_static_fld[j1] = (inputstreamread() & 0xff) << 0;
            b_int_array1d_static_fld[j1] |= (inputstreamread() & 0xff) << 8;
            b_int_array1d_static_fld[j1] |= (inputstreamread() & 0xff) << 16;
            b_int_array1d_static_fld[j1] |= (inputstreamread() & 0xff) << 24;
        }

        q_boolean_static_fld = inputstreamread() == 255;
        a_int_static_fld = inputstreamread();
        for(int k1 = 0; k1 < e_int_array1d_static_fld_length; k1++)
            e_int_array1d_static_fld[k1] = inputstreamread() & 0xff;

        S = inputstreamread() & 0xff;
        R = inputstreamread() & 0xff;
        o_int_static_fld = inputstreamread() & 0xff;
        e_int_static_fld = inputstreamread() & 0xff;
        V = inputstreamread() & 0xff;
        D = (inputstreamread() & 0xff) << 0;
        D |= (inputstreamread() & 0xff) << 8;
        D |= (inputstreamread() & 0xff) << 16;
        D |= (inputstreamread() & 0xff) << 24;
        y = (inputstreamread() & 0xff) << 0;
        y |= (inputstreamread() & 0xff) << 8;
        y |= (inputstreamread() & 0xff) << 16;
        y |= (inputstreamread() & 0xff) << 24;
        c_int_static_fld = inputstreamread() & 0xff;
        ab = inputstreamread() & 0xff;
        q_int_static_fld = inputstreamread() & 0xff;
        j_int_static_fld = inputstreamread() & 0xff;
        for(int l1 = 0; l1 < i_int_array1d_static_fld_length; l1++) {
            i_int_array1d_static_fld[l1] = (inputstreamread() & 0xff) << 0;
            i_int_array1d_static_fld[l1] |= (inputstreamread() & 0xff) << 8;
            i_int_array1d_static_fld[l1] |= (inputstreamread() & 0xff) << 16;
            i_int_array1d_static_fld[l1] |= (inputstreamread() & 0xff) << 24;
        }

        for(int i2 = 0; i2 < l_int_array1d_static_fld_length; i2++) {
            l_int_array1d_static_fld[i2] = (inputstreamread() & 0xff) << 0;
            l_int_array1d_static_fld[i2] |= (inputstreamread() & 0xff) << 8;
            l_int_array1d_static_fld[i2] |= (inputstreamread() & 0xff) << 16;
            l_int_array1d_static_fld[i2] |= (inputstreamread() & 0xff) << 24;
        }

        short c1 = 0x2000;//'\u2000';
        if(E == 0)
            c1 = 0x4000;//'\u4000';
        for(int j2 = 0; j2 < c1; j2++) {
            vram[j2+ram] = (byte)inputstreamread();
            if((j2 & 8) != 0 && j2 >= 8192) {
                int l2 = vram[ram+(j2 ^ 8)] & 0xff;
                int i3 = vram[ram+j2] & 0xff;
                int j3 = ((j2 & 0xfffffff0) >> 1 | (j2 & 7)) - 4096;
                a_char_array1d_static_fld[j3] = (short)(
                    (l2 & 1)         |
                    (l2 & 2) << 1    |
                    (l2 & 4) << 2    |
                    (l2 & 8) << 3    |
                    (l2 & 0x10) << 4 |
                    (l2 & 0x20) << 5 |
                    (l2 & 0x40) << 6 |
                    (l2 & 0x80) << 7 |
                    (i3 & 1) << 1    |
                    (i3 & 2) << 2    |
                    (i3 & 4) << 3    |
                    (i3 & 8) << 4    |
                    (i3 & 0x10) << 5 |
                    (i3 & 0x20) << 6 |
                    (i3 & 0x40) << 7 |
                    (i3 & 0x80) << 8
                );
            }
        }

        b_boolean_static_fld = inputstreamread() == 255;
        x = inputstreamread() & 0xff;
        for(int k2 = 0; k2 < d_byte_array1d_static_fld_length; k2++)
            d_byte_array1d_static_fld[k2] = (byte)inputstreamread();

        p_int_static_fld = d_byte_array1d_static_fld[0] & 0xff;
        C = inputstreamread() & 0xff;
        j();
    }

    static void c_out(){//(OutputStream outputstream) throws IOException {
        for(int i1 = 0; i1 < 10240; i1++)
            outputstreamwrite(vram[i1] & 0xff);

        for(int j1 = 0; j1 < b_int_array1d_static_fld_length; j1++) {
            outputstreamwrite(b_int_array1d_static_fld[j1] >> 0 & 0xff);
            outputstreamwrite(b_int_array1d_static_fld[j1] >> 8 & 0xff);
            outputstreamwrite(b_int_array1d_static_fld[j1] >> 16 & 0xff);
            outputstreamwrite(b_int_array1d_static_fld[j1] >> 24 & 0xff);
        }

        outputstreamwrite(q_boolean_static_fld ? 255 : 0);
        outputstreamwrite(a_int_static_fld & 0xff);
        for(int k1 = 0; k1 < e_int_array1d_static_fld_length; k1++)
            outputstreamwrite(e_int_array1d_static_fld[k1] & 0xff);

        outputstreamwrite(S & 0xff);
        outputstreamwrite(R & 0xff);
        outputstreamwrite(o_int_static_fld & 0xff);
        outputstreamwrite(e_int_static_fld & 0xff);
        outputstreamwrite(V & 0xff);
        outputstreamwrite(D >> 0 & 0xff);
        outputstreamwrite(D >> 8 & 0xff);
        outputstreamwrite(D >> 16 & 0xff);
        outputstreamwrite(D >> 24 & 0xff);
        outputstreamwrite(y >> 0 & 0xff);
        outputstreamwrite(y >> 8 & 0xff);
        outputstreamwrite(y >> 16 & 0xff);
        outputstreamwrite(y >> 24 & 0xff);
        outputstreamwrite(c_int_static_fld & 0xff);
        outputstreamwrite(ab & 0xff);
        outputstreamwrite(q_int_static_fld & 0xff);
        outputstreamwrite(j_int_static_fld & 0xff);
        for(int l1 = 0; l1 < i_int_array1d_static_fld_length; l1++) {
            outputstreamwrite(i_int_array1d_static_fld[l1] >> 0 & 0xff);
            outputstreamwrite(i_int_array1d_static_fld[l1] >> 8 & 0xff);
            outputstreamwrite(i_int_array1d_static_fld[l1] >> 16 & 0xff);
            outputstreamwrite(i_int_array1d_static_fld[l1] >> 24 & 0xff);
        }

        for(int i2 = 0; i2 < l_int_array1d_static_fld_length; i2++) {
            outputstreamwrite(l_int_array1d_static_fld[i2] >> 0 & 0xff);
            outputstreamwrite(l_int_array1d_static_fld[i2] >> 8 & 0xff);
            outputstreamwrite(l_int_array1d_static_fld[i2] >> 16 & 0xff);
            outputstreamwrite(l_int_array1d_static_fld[i2] >> 24 & 0xff);
        }

        short c1 = 0x2000;//'\u2000';
        if(E == 0)
            c1 = 0x4000;//'\u4000';
        for(int j2 = 0; j2 < c1; j2++)
            outputstreamwrite(vram[ram+j2] & 0xff);

        outputstreamwrite(b_boolean_static_fld ? 255 : 0);
        outputstreamwrite(x & 0xff);
        for(int k2 = 0; k2 < d_byte_array1d_static_fld_length; k2++)
            outputstreamwrite(d_byte_array1d_static_fld[k2] & 0xff);

        outputstreamwrite(C & 0xff);
    }

    static int bint1int(int i1) {
        switch(w) {
        case 4: // '\004'
            if(e_boolean_static_fld && i1 >= 0 && i1 < 240 && (R & 0x18) != 0 && --I < 0) {
                I = F;
                return 3;
            } else {
                return 0;
            }

        case 6: // '\006'
            if(e_boolean_static_fld) {
                I += 133;
                if(I >= 65535) {
                    I = 0;
                    return 3;
                }
            }
            return 0;
        }
        return 0;
    }

    static void avoid2int(int i1, int j1) {
      int k1,l1,i2,j2,k2;
//label0:
        switch(i1 >> 13) {
        case 0: // '\0'
            vram[i1 & 0x7ff] = (byte)j1;
            break;

        case 1: // '\001'
            j_int_static_fld = j1;
            switch(i1 & 7) {
            case 2: // '\002'
            default:
                goto label0; //break label0;

            case 0: // '\0'
                S = j1;
                e_int_static_fld = (j1 & 0x10) != 0 ? 4 : 0;
                V = (j1 & 8) != 0 ? 4 : 0;
                x = (j1 & 4) != 0 ? 32 : 1;
                y &= 0xf3ff;
                y |= (j1 & 3) << 10;
                goto label0; //break label0;

            case 1: // '\001'
                k1 = R;
                R = j1;
                if((k1 & 0xe0) != (j1 & 0xe0))
                    j();
                goto label0; //break label0;

            case 3: // '\003'
                C = j1;
                ab = j1 & 7;
                goto label0; //break label0;

            case 4: // '\004'
                if(ab >= 8) {
                    if(C >= 8)
                        d_byte_array1d_static_fld[C] = (byte)j1;
                } else {
                    d_byte_array1d_static_fld[ab] = (byte)j1;
                    if(ab == 0)
                        p_int_static_fld = j1;
                }
                C = (C + 1) & 0xff;
                ab = (ab + 1) & 0xff;
                goto label0; //break label0;

            case 5: // '\005'
                b_boolean_static_fld = !b_boolean_static_fld;
                if(b_boolean_static_fld) {
                    y &= 0xffe0;
                    y |= j1 >> 3;
                    c_int_static_fld = j1 & 7;
                } else {
                    y &= 0x8c1f;
                    y |= (j1 & 0xf8) << 2;
                    y &= 0x8fff;
                    y |= (j1 & 7) << 12;
                }
                goto label0; //break label0;

            case 6: // '\006'
                b_boolean_static_fld = !b_boolean_static_fld;
                if(b_boolean_static_fld) {
                    y &= 0xff;
                    y |= (j1 & 0x3f) << 8;
                } else {
                    y &= 0xff00;
                    y |= j1;
                    D = y;
                }
                goto label0; //break label0;

            case 7: // '\007'
                i1 = D & 0x3fff;
                D += x;
                break;
            }
            if(i1 >= 16128) {
                j1 &= 0x3f;
                i1 &= 0x1f;
                if((i1 & 0xf) == 0) {
                    for(i2 = 0; i2 < 5; i2++) {
                        e_int_array1d_static_fld[i2 << 2] = j1;
                        j_int_array1d_static_fld[i2 << 2] = g_int_array1d_static_fld[j1];
                    }

                    break;
                }
                if((i1 & 3) != 0) {
                    e_int_array1d_static_fld[i1] = j1;
                    j_int_array1d_static_fld[i1] = g_int_array1d_static_fld[j1] | (i1 & 3) << 24;
                }
                break;
            }
            if(i1 >= 12288)
                i1 &= 0xefff;
            i1 = i_int_array1d_static_fld[i1 >> 10] + (i1 & 0x3ff);
            if(i1 < 8192) {
                vram[ram+i1] = (byte)j1;
                break;
            }
            if(E != 0)
                break;
            vram[ram+i1] = (byte)j1;
            if((i1 & 8) != 0) {
                byte byte0 = vram[ram+(i1 ^ 8)];
                j2 = j1;
                k2 = ((i1 & 0xfffffff0) >> 1 | (i1 & 7)) - 4096;
                a_char_array1d_static_fld[k2] = (ushort)(
                    (byte0 & 1)         |
                    (byte0 & 2) << 1    |
                    (byte0 & 4) << 2    |
                    (byte0 & 8) << 3    |
                    (byte0 & 0x10) << 4 |
                    (byte0 & 0x20) << 5 |
                    (byte0 & 0x40) << 6 |
                    (byte0 & 0x80) << 7 |
                    (j2 & 1) << 1       |
                    (j2 & 2) << 2       |
                    (j2 & 4) << 3       |
                    (j2 & 8) << 4       |
                    (j2 & 0x10) << 5    |
                    (j2 & 0x20) << 6    |
                    (j2 & 0x40) << 7    |
                    (j2 & 0x80) << 8
                );
            }//ERROR WAS HERE (char)
            break;

        case 2: // '\002'
            if(i1 < 16408) {
                switch(i1 & 0xff) {
                case 0: // '\0'
                case 1: // '\001'
                case 2: // '\002'
                case 3: // '\003'
                case 4: // '\004'
                case 5: // '\005'
                case 6: // '\006'
                case 7: // '\007'
                case 8: // '\b'
                case 9: // '\t'
                case 10: // '\n'
                case 11: // '\013'
                case 12: // '\f'
                case 13: // '\r'
                case 14: // '\016'
                case 15: // '\017'
                case 16: // '\020'
                case 17: // '\021'
                case 18: // '\022'
                case 19: // '\023'
                case 21: // '\025'
                case 23: // '\027'
                default:
                    goto label0; //break label0;

                case 20: // '\024'
                    l1 = j1 << 8;
                    switch(j1 >> 5) {
                    case 0: // '\0'
                        l1 &= 0x7ff;
                        break;

                    case 1: // '\001'
                    case 2: // '\002'
                        return;

                    case 3: // '\003'
                        l1 -= 22528;
                        break;

                    case 4: // '\004'
                    case 5: // '\005'
                    case 6: // '\006'
                    case 7: // '\007'
                        l1 += b_int_array1d_static_fld[l1 >> 13];
                        break;
                    }
                    Systemarraycopy(vram, l1, d_byte_array1d_static_fld, 0, 256);
                    p_int_static_fld = d_byte_array1d_static_fld[0] & 0xff;
                    goto label0; //break label0;

                case 22: // '\026'
                    break;
                }
                if((j1 & 1) != 0)
                    break;
                a_int_static_fld = k_int_static_fld;
                if((a_int_static_fld & 0x30) == 48)
                    a_int_static_fld &= 0xcf;
                if((a_int_static_fld & 0xc0) == 192)
                    a_int_static_fld &= 0x3f;
            } else {
                xxd(i1, j1);
            }
            break;

        case 3: // '\003'
            if(q_boolean_static_fld)
                vram[i1 - 22528] = (byte)j1;
            break;

        default:
            xxf(i1, j1);
            break;
        }
label0:;
    }
/*

    static void avoid1String1bytes(char *s, byte abyte0[]) throws IOException {
        DataOutputStream dataoutputstream = Connector.openDataOutputStream(s);
        dataoutputstream.write(abyte0);
        dataoutputstream.flush();
        dataoutputstream.close();
    }

    void paint(Graphics g) {
        //System.out.println("paint c");
        int i1 = getWidth();
        int j1 = getHeight();
        if(main.b_boolean_fld) {
            g.setColor(0x336666);
            g.fillRect(0, 0, i1, j1);
            main.b_boolean_fld = false;
        }
        if(fmessage!=null) {
          g.setColor(0xffffff);
          g.fillRect(0, 0, i1, j1);
          g.setColor(0);
          Font font = g.getFont();
          //g.drawString(s, i1 - font.stringWidth(s), j1 - font.getHeight(), 20);
          g.drawString(fmessage, 0, 0, 20);
          fmessage=null;
          return;
       }
            Image image = Image.createRGBImage(main.a_int_array1d_fld, main.a_int_fld, main.c_int_fld, false);
            g.drawImage(image, (i1 - main.a_int_fld)>>1, (j1 - main.c_int_fld)>>1, 20);
        if(main.f_boolean_static_fld && main.a_long_static_fld > 0L && c_long_static_fld > 0L && b_long_static_fld > 0L) {
            long l1 = (main.a_long_static_fld * 10000L) / (c_long_static_fld + b_long_static_fld);
            long l2 = (c_long_static_fld * 10L) / b_long_static_fld;
            g.setColor(0xffffff);
            g.drawString("FPS:" + l1 / 10L + "." + l1 % 10L + " R:" + l2 / 10L + "." + l2 % 10L, (i1 - main.a_int_fld) >>1, (j1 - main.c_int_fld) >>1, 20);
        }
    }
    
void debug(char *s, int a, int b){
  new File().debugWrite("0:/vnes","\n"+s+"  "+Integer.toString(a)+" = "+Integer.toString(b));        
}

void savedebug(){
  File file=new File();
  int handle;
  try{
  handle=file.open("0:/vnes2");
  file.write(handle,vram,0,vram.length);
  file.close(handle);
  }catch(IOException ios){}
}

}
*/

//////////38
    /*
const char bmphead[]={
"\x42\x4D\xC6\xB5\x00\x00\x00\x00\x00\x00\x36\x00\x00\x00\x28\x00"
"\x00\x00\x84\x00\x00\x00\xB0\x00\x00\x00\x01\x00\x10\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x12\x0B\x00\x00\x12\x0B\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\xF8\x00\x00\xE0\x07\x00\x00\x1F\x00"
"\x00\x00\x00\x00\x00\x00"};

void savescreen(char *fname){
  FILE *f;
  int i,j,k;
  int r,g,b;
  short scr[132*176], *ss=scr+132*176;
  int *s=main_a_int_array1d_fld;
  for(j=0;j<176;j++){
    for(i=0;i<132;i++){
      k=*s++;
      b=(k>>16)&0xff;
      g=(k>>8)&0xff;
      r=(k)&0xff;
      *--ss=(r>>3<<10)|(g>>3<<5)|(b>>3);
    }
    s+=256-132;
  }
  f=fopen(fname,"wb");
  fwrite(bmphead,sizeof(bmphead),1,f); 
  fwrite(scr,132*176*2,1,f); 
  fclose(f); 
}
*/
//static int nshow=0;
/*
void repaint(){
  if(++nshow==200){
    printf("show ");
    //nshow=0;
    //savescreen("out.bmp");
    //abort();
  }
 //Image image = Image.createRGBImage(main.a_int_array1d_fld, main.a_int_fld, main.c_int_fld, false);
}

int main(int argc, char **argv){
  int i;
  if(argc<2){
    printf("No ROM!\n");
    return 1;
  }
  initnul();
  h();
  if(i=loadrom(*++argv)){
    printf("Error %d\n",i);
    return i;
  }
  run();
  return 0;
}
*/

void initnul(){
  memset(d_byte_array1d_static_fld,0,256);
  memset(b_int_array1d_static_fld,0,8*4);
  memset(j_int_array1d_static_fld,0,32*2);
memset(g_int_array1d_static_fld,0,64*2);
memset(e_int_array1d_static_fld,0,32*4);
memset(i_int_array1d_static_fld,0,12*4);
memset(l_int_array1d_static_fld,0,8*4);
memset(d_byte_array1d_static_fld,0,256);

memset(c_int_array1d_static_fld,0,260*4);
memset(h_int_array1d_static_fld,0,276*4);
memset(f_int_array1d_static_fld,0,276*4);
memset(b_boolean_array1d_static_fld,0,260);
memset(a_boolean_array1d_static_fld,0,276);
  h();
}
/*
void savefile(char *fname, void *buf, int size){
  FILE *f;
  if(f=fopen(fname,"wb")){
    fwrite(buf,size,1,f);
    fclose(f);
  }
}
*/
void freeall(){
  if(a_char_array1d_static_fld) mfree(a_char_array1d_static_fld);
  if(b_byte_array1d_static_fld) mfree(b_byte_array1d_static_fld);
  if(vram) mfree(vram);
  if(screens) mfree(screens);
}
