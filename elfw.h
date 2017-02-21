#pragma once

#include <iostream>
#include <vector>
#include <map>
#include "mkzbase/variant.h"

#include "elfw-base.h"
#include "elfw-draw.h"


namespace elfw {


    namespace containers {

        class OrderedSet {
        public:

            template <typename Seq>
            OrderedSet(const Seq& src)
            {
                // Store the indices for the hashes
                for( size_t i = 0; i < src.size(); ++i ) {
                    const auto hsh = std::hash<typename Seq::value_type>()(src[i]);
                    hashToIndex.insert({hsh, i});
                }
            }

            size_t operator[](const size_t hsh) const {
                if (!contains(hsh)) return 0xbeefbeef;
                return hashToIndex.at(hsh);
            }

            bool contains(const size_t hsh) const {
                return (hashToIndex.count(hsh) > 0);
            }

            using iterator = std::map<std::size_t, std::size_t>::iterator;
            using const_iterator = std::map<std::size_t, std::size_t>::const_iterator;

            const std::map<std::size_t, std::size_t>& hashes() const { return hashToIndex; };

        private:
            // the source collection
            std::map<std::size_t, std::size_t> hashToIndex;

        };



        struct OrderedSetPatch {
            std::size_t hash, idxA, idxB;
        };

        using Patches = std::vector<OrderedSetPatch>;



        void diff(const OrderedSet& a, const OrderedSet& b, Patches& onlyInA, Patches& onlyInB, Patches& reordered, Patches& constant) {
            // TODO: do this more efficiently
            for(const auto& ae : a.hashes()) {
                const auto hsh = ae.first;
                const auto idxA = ae.second;

                // check for reordering
                if (b.contains(hsh)) {
                    const auto idxB = b[hsh];
                    if (idxA != idxB) {
                        reordered.push_back({hsh, idxA, idxB});
                    } else {
                        constant.push_back({hsh, idxA, idxB});
                    }
                    continue;
                }
                onlyInA.push_back({ae.first, ae.second, 0});
            }

            for(const auto& be : b.hashes()) {
                if (a.contains(be.first)) continue;
                onlyInB.push_back({be.first, 0, be.second});
            }

            // TODO: what if inserting reorders stuff?
        }


    }

    // Represents a box wrapping relative coordinates
    struct Div {
        const char* key;
        const Frame<double> frame;

        const std::vector<const Div> childDivs;
        const std::vector<const draw::Command> drawCommands;
    };


    template <typename T>
    struct ViewPatch {

        enum Op { Add, Remove, Reorder };

        const T* inA;
        const T* inB;

        const Frame<double>* frameA;
        const Frame<double>* frameB;

        size_t idxA, idxB;
        Op op;
    };


    template <typename T, typename SeqA, typename SeqB>
    void diffToPatch(SeqA&& a, SeqB&& b, std::vector<ViewPatch<T>>& patches ) {
        using namespace containers;
        using VP = ViewPatch<T>;
        using Op = typename ViewPatch<T>::Op;
        OrderedSet as(a), bs(b);

        Patches inA, inB, reordered, constant;
        diff(as, bs, inA, inB, reordered, constant );

        const auto appendViewPatch = [&](Op op, const Patches& osPatches){
            auto start = patches.size();
            patches.resize( patches.size() + osPatches.size() );
            std::transform(osPatches.begin(), osPatches.end(), &patches[start], [&](const OrderedSetPatch& p){
                const auto& ae = a[p.idxA];
                const auto& be = b[p.idxB];
                return VP{ &ae, &be, &ae.frame, &be.frame, p.idxA, p.idxB, op};
            });
        };

        appendViewPatch(Op::Remove, inA);
        appendViewPatch(Op::Add, inB);
        appendViewPatch(Op::Reorder, reordered);

    };

    void diff(const Div& a, const Div& b, std::vector<ViewPatch<draw::Command>>& patches) {

        auto isFrameEq = (a.frame == b.frame);

        // if the frame is not eq, then we need to re-render every child (as we dont cache for now
        if (!isFrameEq) {
            printf("Frame differs\n");
            return;
        }

        const auto printDiff = [](const char* prefix, const auto& seq) {
            for (auto& del : seq) {
                std::cout << prefix << " " << del.hash << " [ idxA=" << del.idxA << ", idxB=" << del.idxB << "]\n";
            }
        };

        const auto diffChildren = [&](){
            using namespace containers;
            OrderedSet as(a.childDivs), bs(b.childDivs);

            Patches inA, inB, reordered, constant;
            diff(as, bs, inA, inB, reordered, constant );


            printDiff("- ", inA);
            printDiff("+ ", inB);
            printDiff("% ", reordered);


            using stdhelpers::hash_seq;

            // check the children that stayed the same
            for (auto& child : constant) {
                const auto& ac = a.childDivs[child.idxA];
                const auto& bc = b.childDivs[child.idxB];

                // if the hash of evrything is the same, nothing should happen
                if (hash_seq(0, ac.childDivs) == hash_seq(0, bc.childDivs)
                        && hash_seq(0, ac.drawCommands) == hash_seq(0, bc.drawCommands))
                {
                    continue;
                }

                // Otherwise diff to find out what changed
                diff(a.childDivs[child.idxA], b.childDivs[child.idxB], patches);
            }

        };



        diffChildren();
        diffToPatch(a.drawCommands, b.drawCommands, patches);



    }


    template <typename S>
    void debug(S& s, const Div& div, int indent = 0) {

        auto doIndent = [&](){
            for (int i=0; i < indent * 4; ++i)
                s << " ";
        };

        s << "\n";
        doIndent(); s << "Div: '" << div.key << "'  " << div.frame << "\n";

        indent += 1;

        for (auto& cmd : div.drawCommands) {
            using namespace draw;
            doIndent();
            s << cmd;
        }

        for (auto& child : div.childDivs) {
            debug(s,child, indent + 1);
        }
    }

    template <typename S>
    S& operator<<(S& s, const Div& d) {
        debug(s, d, 0);
        return s;
    }
}

MAKE_HASHABLE(elfw::Div, t.frame, t.key )

