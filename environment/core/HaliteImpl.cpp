#include "HaliteImpl.hpp"

namespace hlt {

/**
 * Communicate with bots to obtain commands for next step.
 * @return Commands received from players.
 */
std::unordered_map<id_type, Command> HaliteImpl::retrieve_commands() {
    std::unordered_map<id_type, Command> commands;
    for (auto &player : game->players) {
        Command command;
        game->networking.handle_frame(player.second, command);
        commands[player.first] = std::move(command);
    }
    return commands;
}

/**
 * Process the effects of commands.
 * @param commands The commands to process.
 */
void HaliteImpl::process_commands(const std::unordered_map<id_type, Command> &commands) {
    for (const auto &player_command : commands) {
        auto &player = game->players[player_command.first];
        auto &command = player_command.second;
        command->act_on_map(game->game_map, player);
    }
}

/** Process all entity lifecycle events for this turn. */
void HaliteImpl::process_entities() {
    // TODO: merging the existing entities is being left to the first traversal (production calculation).
    // Each factory may spawn a new entity
    const auto &constants = Constants::get();
    for (auto &player_pair : game->players) {
        auto &player = player_pair.second;
        if (player.energy >= constants.NEW_ENTITY_ENERGY_COST) {
            player.energy -= constants.NEW_ENTITY_ENERGY_COST;
            auto entity = make_entity<Entity>(player.player_id, constants.NEW_ENTITY_ENERGY);
            // TODO: what if there is already an entity on the factory? Merge
            player.entities[player.factory_location] = entity;
        }
    }
    // Each entity loses some health, each entity with no remaining energy is removed
    for (auto &player_pair : game->players) {
        auto &player = player_pair.second;
        auto entity = player.entities.begin();
        while (entity != player.entities.end()) {
            entity->second->energy -= constants.BASE_TURN_ENERGY_LOSS;
            if (entity->second->energy <= 0) {
                player.entities.erase(entity++);
            } else {
                entity++;
            }
        }
    }
}

}
