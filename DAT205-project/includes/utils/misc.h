#pragma once

// --- <TOGGLE THIS FOR DEBUG OUTPUT> ----
#define _DEBUG
// ---  </TOGGLE THIS FOR DEBUG OUTPUT>----

#ifdef _DEBUG
#define print(x) std::cout << x << std::endl
#else
#define print(x) // do nothing
#endif