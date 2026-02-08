#pragma once

class CNEOHud_Ammo;
class CNEOHud_HTA;

struct NeoRootGamehud
{
	CNEOHud_Ammo *ammo;
	CNEOHud_HTA *hta;
};

void NeoRootGamehud_Init(NeoRootGamehud *gamehud);
void NeoRootGamehud_Paint(NeoRootGamehud *gamehud);

