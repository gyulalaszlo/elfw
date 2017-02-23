#include "elfw-viewtree-resolve.h"



namespace {
    using namespace elfw;

    draw::ResolvedCommand resolveCommand(Rect<double> viewRect, const draw::Command& cmd) {
        return { frame::resolve(cmd.frame, viewRect), cmd.cmd };
    }


    template<typename Divs>
    void resolveRec(
            Rect<double> viewRect,
            Divs&& divs,
            draw::ResolvedCommandList& commandList,
            std::vector<ResolvedDiv>& divList
    ) {
        mkz::tree_to_linear_map<ResolvedDiv>(
                divs, divList, viewRect,
                [&](auto&& frameRect, const Div& div, auto&& recurse) {

                    // resolve commands
                    auto cmds_slice = mkz::indexed_slice_from_append<draw::ResolvedCommand>(
                            div.drawCommands.begin(), div.drawCommands.end(),
                            commandList,
                            [&](auto&& cmd) {
                                return resolveCommand(frameRect, cmd);
                            }).slice();

                    // resolve divs
                    return ResolvedDiv{
                            div.key,
                            frameRect,
                            cmds_slice,
                            // children indices
                            recurse(frame::resolve(div.frame, frameRect), div.childDivs)
                    };
                }
        );
    }

}


namespace elfw {
// Converts a Div tree to ResolvedDivs and hashes all data in the tree
    ViewTreeWithHashes resolveDiv(Rect<double> viewRect, const Div& div) {
        auto v = ViewTreeWithHashes { };
        resolveRec(viewRect, std::vector<Div>{div}, v.drawCommands, v.divs);
        updateViewTreeHashes(v.divs[0], v.hashStore, v.drawCommands, v.divs);
        return v;
    }

}
