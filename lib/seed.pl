#
# -*- Perl -*-
# $Id: seed.pl,v 1.2 1999-09-07 00:58:59 satoru Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi  All rights reserved.
#     This is free software with ABSOLUTELY NO WARRANTY.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either versions 2, or (at your option)
#  any later version.
# 
#  This program is distributed in the hope that it will be useful
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA
#
#  This file must be encoded in EUC-JP encoding

package seed;

#
# Dr. Knuth's ``hash'' (from UNIX MAGAZINE May, 1998)
#
sub init () {
    return (
	 [
	  3852, 26205, 51350, 2876, 47217, 47194, 55549, 43312, 
	  63689, 40984, 62703, 10954, 13108, 60460, 41680, 32277, 
	  51887, 28590, 17502, 57168, 37798, 27466, 13800, 12816, 
	  53745, 8833, 55089, 15481, 18993, 15262, 8490, 22846, 
	  41468, 59841, 25722, 23150, 41499, 15735, 926, 39653, 
	  56720, 63629, 50607, 4292, 58554, 26752, 36570, 44905, 
	  55343, 54073, 36538, 27605, 16003, 50339, 40422, 4213, 
	  59172, 29975, 19694, 12629, 45238, 28185, 35475, 21170, 
	  22491, 61198, 44320, 63991, 11398, 45247, 38108, 2583, 
	  43341, 23180, 6875, 36359, 49933, 43446, 15728, 39740, 
	  31983, 52267, 1809, 47986, 37070, 42232, 52199, 30706, 
	  6672, 6358, 43336, 51910, 34544, 13276, 7545, 57036, 
	  8939, 51866, 55491, 20338, 31577, 28064, 22921, 9383, 
	  51245, 29797, 45742, 35642, 7707, 61471, 9847, 39691, 
	  48202, 11656, 22141, 19736, 53889, 8805, 50443, 60561, 
	  15164, 28244, 46936, 49709, 41521, 54481, 41209, 50460, 
	  40812, 31165, 5262, 6853, 59230, 28184, 16237, 44940, 
	  57981, 61979, 15046, 152, 57914, 24893, 39843, 40581, 
	  36550, 61985, 60318, 24904, 5255, 45226, 19929, 20420, 
	  7934, 1329, 4593, 49456, 55811, 45803, 34381, 31087, 
	  11433, 39644, 37941, 5128, 2292, 54178, 50068, 60273, 
	  50622, 65115, 60426, 43000, 24473, 34734, 18046, 61024, 
	  31184, 12828, 20392, 36439, 58054, 40322, 56860, 453, 
	  41651, 61453, 49909, 31927, 41721, 18754, 63015, 53155, 
	  58398, 35421, 58283, 60691, 24063, 42816, 55428, 9149, 
	  42395, 50319, 52150, 1332, 19517, 4661, 62357, 50701, 
	  17489, 17213, 21605, 10008, 57535, 12929, 10462, 33651, 
	  8847, 60371, 43, 50569, 13590, 63058, 38188, 6453, 
	  32943, 30936, 1608, 57007, 8216, 57037, 621, 50611, 
	  41820, 52771, 51944, 61338, 57433, 48765, 46504, 9387, 
	  443, 2573, 19395, 57978, 15503, 29857, 26094, 24351, 
	  24693, 26137, 9385, 38284, 23659, 47573, 44738, 56602
	  ],
	 [
	  12974, 46347, 48074, 21190, 37848, 48695, 6266, 14133, 
	  35931, 58211, 9935, 27828, 41440, 56440, 37215, 41883, 
	  59014, 56610, 34326, 8982, 20932, 60420, 33333, 45626, 
	  21021, 42718, 18375, 44681, 24756, 63113, 35748, 37730, 
	  43924, 18286, 58920, 1445, 65187, 30371, 37376, 57862, 
	  40307, 65205, 33766, 31211, 36884, 10114, 24689, 27959, 
	  44441, 33671, 48892, 39326, 1469, 28982, 60348, 44188, 
	  47357, 39493, 3408, 44935, 9705, 41138, 23324, 27992, 
	  34523, 39562, 29437, 34174, 4397, 1278, 26500, 44705, 
	  947, 60267, 10380, 37832, 4846, 35070, 255, 49288, 
	  3206, 49147, 23078, 4676, 12594, 17890, 48864, 59951, 
	  57383, 52273, 39351, 1553, 27875, 62675, 29545, 62399, 
	  36701, 58983, 31038, 41099, 60262, 57539, 20268, 61210, 
	  52271, 30649, 33506, 57118, 184, 33762, 40870, 3390, 
	  17374, 63949, 8067, 29968, 16303, 56931, 24384, 8151, 
	  43668, 63736, 6008, 60875, 39251, 2872, 32040, 32699, 
	  33910, 7603, 27426, 25914, 27872, 23100, 12649, 58521, 
	  56607, 4231, 58705, 24834, 45102, 62096, 42208, 43515, 
	  4627, 6641, 59819, 61559, 31026, 2435, 39692, 29226, 
	  12141, 45700, 24565, 51392, 48573, 56606, 18556, 16947, 
	  64210, 45982, 42861, 26546, 3546, 55511, 19531, 60154, 
	  59743, 12700, 19452, 39309, 9261, 61660, 17289, 13888, 
	  2766, 11572, 9912, 33792, 14008, 49604, 63018, 26149, 
	  29769, 22048, 12006, 12806, 13118, 30562, 29754, 11792, 
	  11008, 7080, 38339, 14554, 62591, 57870, 9172, 56798, 
	  5035, 28625, 30572, 14297, 24749, 47861, 27515, 59433, 
	  38098, 61308, 7906, 22166, 58790, 34055, 51935, 15303, 
	  46061, 64742, 28421, 11087, 28960, 40214, 22095, 36041, 
	  13018, 36650, 33096, 5352, 45823, 24359, 10388, 8912, 
	  54931, 24685, 33662, 37257, 52871, 61178, 31155, 25433, 
	  56950, 39061, 47599, 50204, 7580, 33999, 65507, 53642, 
	  33205, 28393, 64730, 62166, 3072, 21290, 32671, 16090
	  ],
	 [
	  57940, 232, 21443, 38228, 24592, 31831, 47141, 13988, 
	  56517, 15268, 43852, 10910, 16864, 3750, 2324, 55926, 
	  52529, 63507, 19813, 52501, 51613, 53019, 15359, 50807, 
	  49650, 18431, 6561, 16785, 34522, 64502, 17018, 55965, 
	  37195, 41610, 22261, 18801, 55598, 13243, 34069, 41307, 
	  57095, 44979, 58172, 60846, 47304, 48562, 46660, 34298, 
	  46533, 938, 21264, 32611, 53957, 36623, 17883, 38072, 
	  55055, 24444, 54857, 24042, 23411, 6340, 14471, 60606, 
	  47950, 36733, 13872, 38012, 49976, 47941, 13784, 41536, 
	  27385, 6421, 36846, 9154, 54984, 17971, 43452, 35982, 
	  18909, 64716, 3057, 7331, 35804, 20941, 45403, 25324, 
	  45385, 34725, 49366, 3261, 41065, 63838, 63868, 23479, 
	  35036, 12204, 61492, 19476, 60146, 9741, 61013, 21995, 
	  16163, 32324, 31149, 5612, 50295, 9066, 41594, 3669, 
	  8247, 44652, 11000, 44052, 57, 56404, 3840, 45443, 
	  25593, 53206, 48704, 1123, 51508, 47037, 24603, 21008, 
	  59241, 20559, 40485, 53851, 30301, 35963, 10311, 46465, 
	  2751, 41461, 52077, 53047, 50527, 28135, 56717, 58775, 
	  7252, 2182, 37291, 7309, 58586, 41131, 52753, 18644, 
	  28802, 35922, 19767, 14775, 17423, 44371, 35784, 11128, 
	  64931, 10734, 64980, 29696, 46697, 9756, 10626, 49449, 
	  51217, 36961, 36209, 25303, 28142, 29448, 32555, 30324, 
	  1204, 39865, 23375, 42336, 27082, 42020, 5602, 63004, 
	  61788, 20378, 14892, 40623, 56162, 26021, 40018, 1360, 
	  25466, 4179, 48058, 35222, 14805, 31971, 20903, 11973, 
	  3396, 57112, 37276, 31539, 21025, 4295, 61864, 22230, 
	  44161, 19704, 64566, 5707, 61724, 4633, 3176, 57977, 
	  25011, 18069, 33064, 15638, 44090, 7547, 16998, 4020, 
	  11727, 65056, 39242, 26532, 31492, 38506, 34888, 51723, 
	  10246, 891, 7213, 14542, 62756, 29443, 58703, 16924, 
	  28473, 64411, 13112, 33107, 2052, 5554, 58118, 20121, 
	  38618, 8220, 64212, 46166, 25219, 2696, 57893, 24740
	  ],
	 [
	  41939, 18890, 56232, 36549, 57396, 25584, 22736, 2106, 
	  26476, 29949, 16648, 23697, 59393, 9816, 40621, 22331, 
	  8691, 53734, 55438, 10743, 59288, 48021, 30865, 32371, 
	  56242, 29541, 13001, 15925, 32237, 5358, 40666, 8641, 
	  24249, 31362, 45191, 16109, 56947, 2391, 18216, 17887, 
	  32341, 34864, 41584, 26199, 44680, 16670, 48530, 53372, 
	  4868, 38432, 64115, 64156, 20918, 29445, 30992, 11624, 
	  58986, 43993, 27550, 25688, 49352, 2680, 34329, 8065, 
	  34042, 13984, 24174, 25454, 16376, 42391, 43342, 48718, 
	  11719, 19390, 9381, 56400, 36061, 57911, 44237, 40929, 
	  30808, 39550, 51726, 6725, 5006, 63351, 176, 49000, 
	  25365, 25864, 32816, 28046, 60193, 40882, 62089, 8642, 
	  65057, 22007, 25018, 41912, 65349, 8201, 53632, 19204, 
	  17582, 44496, 55265, 9957, 23197, 30659, 40765, 478, 
	  4674, 26956, 7204, 9681, 24771, 7380, 58681, 50137, 
	  33245, 25962, 12647, 27903, 1308, 9200, 36545, 829, 
	  31207, 61564, 42741, 31021, 4229, 30837, 50225, 21812, 
	  9798, 39955, 31769, 32996, 5078, 6999, 33475, 9753, 
	  33956, 40679, 19434, 58727, 48060, 12579, 43328, 15770, 
	  38541, 55975, 43673, 39849, 65176, 14683, 30848, 10711, 
	  17884, 61869, 14941, 48722, 46559, 36753, 58520, 20978, 
	  2987, 25981, 26057, 9987, 59456, 35810, 43943, 34600, 
	  55244, 37135, 17124, 2288, 14928, 32895, 40829, 5368, 
	  11032, 15143, 5008, 25715, 55822, 35856, 36427, 8171, 
	  32190, 51369, 56893, 13214, 22587, 49878, 34193, 25575, 
	  10323, 60250, 35562, 4243, 30525, 13970, 38843, 20234, 
	  51106, 55968, 22523, 498, 23327, 63352, 5866, 34360, 
	  12960, 10874, 60076, 3247, 46731, 30967, 11418, 13386, 
	  16801, 2776, 26600, 39388, 52654, 60793, 64963, 62978, 
	  55508, 34990, 1686, 20498, 48960, 40530, 40733, 34530, 
	  30962, 63256, 35029, 54290, 61073, 40895, 23115, 8497, 
	  51770, 17655, 11744, 32966, 48622, 23162, 46352, 65423
	  ]
	    );
}

1;
