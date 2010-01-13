#include "AveragingManager.h"
#include "AveragingGroup.h"

namespace scag2 {
namespace counter {

void AverageItem::addItem( AveragingGroup& grp )
{
    grp.addItem(*this);
}
void AverageItem::remItem( AveragingGroup& grp )
{
    grp.remItem(*this);
}

}
}
